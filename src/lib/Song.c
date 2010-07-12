

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <Connections_search.h>
#include <Random.h>
#include <Real.h>
#include <Song.h>
#include <File_base.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


Song* new_Song(int buf_count, uint32_t buf_size)
{
    assert(buf_count >= 1);
    assert(buf_count <= KQT_BUFFERS_MAX);
    assert(buf_size > 0);
    assert(buf_size <= KQT_BUFFER_SIZE_MAX);
    Song* song = xalloc(Song);
    if (song == NULL)
    {
        return NULL;
    }
    if (!Device_init(&song->parent, buf_size))
    {
        xfree(song);
        return NULL;
    }
    Device_register_port(&song->parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    song->buf_count = buf_count;
    song->buf_size = buf_size;
    song->priv_bufs[0] = NULL;
    song->voice_bufs[0] = NULL;
    song->voice_bufs2[0] = NULL;
    song->subsongs = NULL;
    song->pats = NULL;
    song->insts = NULL;
    song->dsps = NULL;
    song->connections = NULL;
    song->play_state = NULL;
    song->event_handler = NULL;
    song->skip_state = NULL;
    song->skip_handler = NULL;
    song->random = NULL;
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
    {
        song->scales[i] = NULL;
    }
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        song->bufs[i] = NULL;
    }
    for (int i = 0; i < buf_count; ++i)
    {
        song->priv_bufs[i] = xnalloc(kqt_frame, buf_size);
        if (song->priv_bufs[i] == NULL)
        {
            del_Song(song);
            return NULL;
        }
        for (uint32_t k = 0; k < buf_size; ++k)
        {
            song->priv_bufs[i][k] = 0;
        }
        song->bufs[i] = song->priv_bufs[i];
        song->voice_bufs[i] = xnalloc(kqt_frame, buf_size);
        if (song->voice_bufs[i] == NULL)
        {
            del_Song(song);
            return NULL;
        }
        song->voice_bufs2[i] = xnalloc(kqt_frame, buf_size);
        if (song->voice_bufs2[i] == NULL)
        {
            del_Song(song);
            return NULL;
        }
    }
    song->random = new_Random();
    if (song->random == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->subsongs = new_Subsong_table();
    if (song->subsongs == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->pats = new_Pat_table(KQT_PATTERNS_MAX);
    if (song->pats == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->insts = new_Ins_table(KQT_INSTRUMENTS_MAX);
    if (song->insts == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->dsps = new_DSP_table(KQT_DSP_EFFECTS_MAX);
    if (song->dsps == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->scales[0] = new_Scale(SCALE_DEFAULT_REF_PITCH,
            SCALE_DEFAULT_OCTAVE_RATIO);
    if (song->scales[0] == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->play_state = new_Playdata(song->insts,
                                    song->random);
    if (song->play_state == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->play_state->subsongs = Song_get_subsongs(song);
    song->play_state->scales = song->scales;
    song->play_state->active_scale = &song->play_state->scales[0];
    song->skip_state = new_Playdata_silent(1000000000);
    if (song->skip_state == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->skip_state->subsongs = Song_get_subsongs(song);

    if (!Device_init_buffer(&song->parent, DEVICE_PORT_TYPE_RECEIVE, 0))
    {
        del_Song(song);
        return NULL;
    }
    Read_state* conn_state = READ_STATE_AUTO;
    song->connections = new_Connections_from_string(NULL, false, conn_state);
    if (song->connections == NULL)
    {
        assert(!conn_state->error);
        del_Song(song);
        return NULL;
    }
    if (!Connections_prepare(song->connections,
                             &song->parent,
                             song->insts))
    {
        del_Song(song);
        return NULL;
    }

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        song->channels[i] = new_Channel(song->insts, i,
                                        song->play_state->voice_pool,
                                        &song->play_state->tempo,
                                        &song->play_state->freq);
        if (song->channels[i] == NULL)
        {
            del_Song(song);
            return NULL;
        }
    }
    Channel_state* ch_states[KQT_COLUMNS_MAX] = { NULL };
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        ch_states[i] = &song->channels[i]->cur_state;
    }

    song->event_handler = new_Event_handler(song->play_state,
                                            ch_states,
                                            song->insts);
    if (song->event_handler == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->skip_handler = new_Event_handler(song->skip_state,
                                           ch_states,
                                           song->insts);
    if (song->skip_handler == NULL)
    {
        del_Song(song);
        return NULL;
    }

    if (Scale_ins_note(song->scales[0], 0,
                       Real_init_as_frac(REAL_AUTO, 1, 1)) < 0)
    {
        del_Song(song);
        return NULL;
    }
    for (int i = 1; i < 12; ++i)
    {
        if (Scale_ins_note_cents(song->scales[0], i, i * 100) < 0)
        {
            del_Song(song);
            return NULL;
        }
    }
    song->mix_vol_dB = SONG_DEFAULT_MIX_VOL;
    song->mix_vol = exp2(song->mix_vol_dB / 6);
    song->init_subsong = SONG_DEFAULT_INIT_SUBSONG;
    return song;
}


bool Song_parse_composition(Song* song, char* str, Read_state* state)
{
    assert(song != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    int64_t buf_count = SONG_DEFAULT_BUF_COUNT;
    double mix_vol = SONG_DEFAULT_MIX_VOL;
    int64_t init_subsong = SONG_DEFAULT_INIT_SUBSONG;
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            bool expect_key = true;
            while (expect_key)
            {
                char key[128] = { '\0' };
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (strcmp(key, "mix_vol") == 0)
                {
                    str = read_double(str, &mix_vol, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (!isfinite(mix_vol) && mix_vol != -INFINITY)
                    {
                        Read_state_set_error(state,
                                 "Invalid mixing volume: %f", song->mix_vol_dB);
                        return false;
                    }
                }
                else if (strcmp(key, "init_subsong") == 0)
                {
                    str = read_int(str, &init_subsong, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (init_subsong < 0 || init_subsong >= KQT_SUBSONGS_MAX)
                    {
                        Read_state_set_error(state,
                                 "Invalid initial Subsong number: %" PRId64,
                                 init_subsong);
                        return false;
                    }
                }
                else
                {
                    Read_state_set_error(state,
                             "Unrecognised key in composition info: %s", key);
                    return false;
                }
                if (state->error)
                {
                    return false;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    if (!Song_set_buf_count(song, buf_count))
    {
        Read_state_set_error(state,
                "Couldn't allocate memory for mixing buffers");
        return false;
    }
    song->mix_vol_dB = mix_vol;
    song->mix_vol = exp2(song->mix_vol_dB / 6);
    Song_set_subsong(song, init_subsong);
    return true;
}


uint32_t Song_mix(Song* song, uint32_t nframes, Event_handler* eh)
{
    assert(song != NULL);
    assert(eh != NULL);
    Playdata* play = Event_handler_get_global_state(eh);
    if (play->mode == STOP)
    {
        return 0;
    }
    if (nframes > song->buf_size && !play->silent)
    {
        nframes = song->buf_size;
    }
    if (!play->silent && song->connections != NULL)
    {
        Connections_clear_buffers(song->connections, 0, nframes);
        for (int i = 0; i < song->buf_count; ++i)
        {
            for (uint32_t k = 0; k < nframes; ++k)
            {
                song->voice_bufs[i][k] = 0;
                song->voice_bufs2[i][k] = 0;
            }
        }
    }
    uint32_t mixed = 0;
    while (mixed < nframes && play->mode)
    {
        Pattern* pat = NULL;
        if (play->mode >= PLAY_SUBSONG)
        {
            int16_t pat_index = KQT_SECTION_NONE;
            Subsong* ss = Subsong_table_get(song->subsongs, play->subsong);
            if (ss != NULL)
            {
                pat_index = Subsong_get(ss, play->section);
            }
            if (pat_index >= 0)
            {
                pat = Pat_table_get(song->pats, pat_index);
            }
        }
        else if (play->mode == PLAY_PATTERN && play->pattern >= 0)
        {
            pat = Pat_table_get(song->pats, play->pattern);
        }
        if (pat == NULL && play->mode != PLAY_EVENT)
        {
            if (play->mode < PLAY_SONG)
            {
                play->mode = STOP;
                break;
            }
            assert(play->mode == PLAY_SONG);
            if (play->subsong >= KQT_SUBSONGS_MAX - 1)
            {
                Playdata_set_subsong(play, 0);
                play->mode = STOP;
                break;
            }
            Playdata_set_subsong(play, play->subsong + 1);
            continue;
        }
        mixed += Pattern_mix(pat, nframes, mixed, eh, song->channels,
                             song->connections);
    }
    Audio_buffer* buffer = NULL;
    if (song->connections != NULL)
    {
        Device* master = Device_node_get_device(
                                 Connections_get_master(song->connections));
        buffer = Device_get_buffer(master,
                         DEVICE_PORT_TYPE_RECEIVE, 0);
    }
    if (!play->silent && buffer != NULL)
    {
        kqt_frame* bufs[] =
        {
            Audio_buffer_get_buffer(buffer, 0),
            Audio_buffer_get_buffer(buffer, 1),
        };
        for (int i = 0; i < song->buf_count; ++i)
        {
            for (uint32_t k = 0; k < mixed; ++k)
            {
                bufs[i][k] *= song->mix_vol;
                if (bufs[i][k] < play->min_amps[i])
                {
                    play->min_amps[i] = bufs[i][k];
                }
                if (bufs[i][k] > play->max_amps[i])
                {
                    play->max_amps[i] = bufs[i][k];
                }
                if (fabs(bufs[i][k]) > 1)
                {
                    ++play->clipped[i];
                }
            }
        }
    }
    play->play_frames += mixed;
#if 0
    fprintf(stderr, "%3d  \r", play->active_voices);
    play->active_voices = 0;
#endif
    return mixed;
}


uint64_t Song_skip(Song* song, Event_handler* eh, uint64_t amount)
{
    assert(song != NULL);
    assert(eh != NULL);
    Playdata* play = Event_handler_get_global_state(eh);
    bool orig_silent = play->silent;
    play->silent = true;
    uint64_t mixed = 0;
    while (mixed < amount)
    {
        uint64_t max_mix = amount - mixed;
        uint64_t nframes = MIN(max_mix, play->freq);
        uint32_t inc = Song_mix(song, nframes, eh);
        mixed += inc;
        if (inc < Song_get_buf_size(song))
        {
            break;
        }
    }
    play->silent = orig_silent;
    return mixed;
}


void Song_set_mix_vol(Song* song, double mix_vol)
{
    assert(song != NULL);
    assert(isfinite(mix_vol) || mix_vol == -INFINITY);
    song->mix_vol_dB = mix_vol;
    song->mix_vol = exp2(mix_vol / 6);
    return;
}


double Song_get_mix_vol(Song* song)
{
    assert(song != NULL);
    return song->mix_vol_dB;
}


void Song_set_subsong(Song* song, uint16_t num)
{
    assert(song != NULL);
    assert(num < KQT_SUBSONGS_MAX);
    song->init_subsong = num;
    return;
}


uint16_t Song_get_subsong(Song* song)
{
    assert(song != NULL);
    return song->init_subsong;
}


bool Song_set_buf_count(Song* song, int count)
{
    assert(song != NULL);
    assert(count > 0);
    assert(count <= KQT_BUFFERS_MAX);
    if (song->buf_count == count)
    {
        return true;
    }
    else if (count < song->buf_count)
    {
        for (int i = count; i < song->buf_count; ++i)
        {
            xfree(song->priv_bufs[i]);
            song->priv_bufs[i] = song->bufs[i] = NULL;
            xfree(song->voice_bufs[i]);
            song->voice_bufs[i] = NULL;
            xfree(song->voice_bufs2[i]);
            song->voice_bufs2[i] = NULL;
        }
        // TODO: remove Instrument buffers
    }
    else
    {
        for (int i = song->buf_count; i < count; ++i)
        {
            song->priv_bufs[i] = xnalloc(kqt_frame, song->buf_size);
            if (song->priv_bufs[i] == NULL)
            {
                return false;
            }
            song->bufs[i] = song->priv_bufs[i];
            song->voice_bufs[i] = xnalloc(kqt_frame, song->buf_size);
            if (song->voice_bufs[i] == NULL)
            {
                return false;
            }
            song->voice_bufs2[i] = xnalloc(kqt_frame, song->buf_size);
            if (song->voice_bufs2[i] == NULL)
            {
                return false;
            }
        }
        // TODO: create Instrument buffers
    }
    song->buf_count = count;
    return true;
}


int Song_get_buf_count(Song* song)
{
    assert(song != NULL);
    return song->buf_count;
}


bool Song_set_buf_size(Song* song, uint32_t size)
{
    assert(song != NULL);
    assert(size > 0);
    if (song->buf_size == size)
    {
        return true;
    }
    for (int i = 0; i < song->buf_count; ++i)
    {
        kqt_frame* new_buf = xrealloc(kqt_frame, size, song->priv_bufs[i]);
        if (new_buf == NULL)
        {
            if (i == 0) // first change failed -- keep the original state
            {
                return false;
            }
            if (size < song->buf_size)
            {
                song->buf_size = size;
            }
            return false;
        }
        song->priv_bufs[i] = new_buf;
        song->bufs[i] = song->priv_bufs[i];

        new_buf = xrealloc(kqt_frame, size, song->voice_bufs[i]);
        if (new_buf == NULL)
        {
            if (size < song->buf_size)
            {
                song->buf_size = size;
            }
            return false;
        }
        song->voice_bufs[i] = new_buf;

        new_buf = xrealloc(kqt_frame, size, song->voice_bufs2[i]);
        if (new_buf == NULL)
        {
            if (size < song->buf_size)
            {
                song->buf_size = size;
            }
            return false;
        }
        song->voice_bufs2[i] = new_buf;
    }
    // TODO: resize Instrument buffers
    song->buf_size = size;
    return true;
}


uint32_t Song_get_buf_size(Song* song)
{
    assert(song != NULL);
    return song->buf_size;
}


kqt_frame** Song_get_bufs(Song* song)
{
    assert(song != NULL);
    return song->bufs;
}


kqt_frame** Song_get_voice_bufs(Song* song)
{
    assert(song != NULL);
    return song->voice_bufs;
}


kqt_frame** Song_get_voice_bufs2(Song* song)
{
    assert(song != NULL);
    return song->voice_bufs2;
}


Subsong_table* Song_get_subsongs(Song* song)
{
    assert(song != NULL);
    return song->subsongs;
}


Pat_table* Song_get_pats(Song* song)
{
    assert(song != NULL);
    return song->pats;
}


Ins_table* Song_get_insts(Song* song)
{
    assert(song != NULL);
    return song->insts;
}


DSP_table* Song_get_dsps(Song* song)
{
    assert(song != NULL);
    return song->dsps;
}


Scale** Song_get_scales(Song* song)
{
    assert(song != NULL);
    return song->play_state->scales;
}


Scale* Song_get_scale(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    return song->scales[index];
}


void Song_set_scale(Song* song, int index, Scale* scale)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    assert(scale != NULL);
    if (song->scales[index] != NULL &&
            song->scales[index] != scale)
    {
        del_Scale(song->scales[index]);
    }
    song->scales[index] = scale;
    return;
}


Scale*** Song_get_active_scale(Song* song)
{
    assert(song != NULL);
    return &song->play_state->active_scale;
}


bool Song_create_scale(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    if (song->scales[index] != NULL)
    {
        Scale_clear(song->scales[index]);
        return true;
    }
    song->scales[index] = new_Scale(SCALE_DEFAULT_REF_PITCH,
            SCALE_DEFAULT_OCTAVE_RATIO);
    if (song->scales[index] == NULL)
    {
        return false;
    }
    return true;
}


void Song_remove_scale(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    if (song->scales[index] != NULL)
    {
        del_Scale(song->scales[index]);
        song->scales[index] = NULL;
    }
    return;
}


void del_Song(Song* song)
{
    assert(song != NULL);
    for (int i = 0; i < song->buf_count && song->priv_bufs[i] != NULL; ++i)
    {
        xfree(song->priv_bufs[i]);
    }
    for (int i = 0; i < song->buf_count && song->voice_bufs[i] != NULL; ++i)
    {
        xfree(song->voice_bufs[i]);
    }
    for (int i = 0; i < song->buf_count && song->voice_bufs2[i] != NULL; ++i)
    {
        xfree(song->voice_bufs2[i]);
    }
    if (song->subsongs != NULL)
    {
        del_Subsong_table(song->subsongs);
    }
    if (song->pats != NULL)
    {
        del_Pat_table(song->pats);
    }
    if (song->insts != NULL)
    {
        del_Ins_table(song->insts);
    }
    if (song->dsps != NULL)
    {
        del_DSP_table(song->dsps);
    }
    if (song->connections != NULL)
    {
        del_Connections(song->connections);
    }
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
    {
        if (song->scales[i] != NULL)
        {
            del_Scale(song->scales[i]);
        }
    }
    if (song->play_state != NULL)
    {
        del_Playdata(song->play_state);
    }
    if (song->skip_state != NULL)
    {
        del_Playdata(song->skip_state);
    }
    for (int i = 0; i < KQT_COLUMNS_MAX && song->channels[i] != NULL; ++i)
    {
        del_Channel(song->channels[i]);
    }
    if (song->event_handler != NULL)
    {
        del_Event_handler(song->event_handler);
    }
    if (song->skip_handler != NULL)
    {
        del_Event_handler(song->skip_handler);
    }
    if (song->random != NULL)
    {
        del_Random(song->random);
    }
    Device_uninit(&song->parent);
    xfree(song);
    return;
}


