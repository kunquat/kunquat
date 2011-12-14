

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


/**
 * Resets the Song.
 *
 * \param device   The Song Device -- must not be \c NULL.
 */
static void Song_reset(Device* device);


/**
 * Sets the master random seed of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param seed   The random seed.
 */
static void Song_set_random_seed(Song* song, uint64_t seed);


/**
 * Sets the mixing rate of the Song.
 *
 * This function sets the mixing rate for all the Instruments and Effects.
 *
 * \param device     The Song Device -- must not be \c NULL.
 * \param mix_rate   The mixing frequency -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Song_set_mix_rate(Device* device, uint32_t mix_rate);


/**
 * Sets the buffer size of the Song.
 *
 * This function sets the buffer size for all the Instruments and Effects.
 *
 * \param device   The Song Device -- must not be \c NULL.
 * \param size     The new buffer size -- must be > \c 0 and
 *                 <= \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Song_set_buffer_size(Device* device, uint32_t size);


/**
 * Synchronises the Song.
 *
 * This function synchronises all the Devices the Song contains. It should be
 * called after loading a Kunquat composition.
 *
 * \param device   The Song Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Song_sync(Device* device);


Song* new_Song(uint32_t buf_size)
{
    assert(buf_size > 0);
    assert(buf_size <= KQT_BUFFER_SIZE_MAX);
    Song* song = xalloc(Song);
    if (song == NULL)
    {
        return NULL;
    }
    if (!Device_init(&song->parent, buf_size, 48000))
    {
        xfree(song);
        return NULL;
    }
    Device_set_reset(&song->parent, Song_reset);
    Device_set_mix_rate_changer(&song->parent, Song_set_mix_rate);
    Device_set_buffer_size_changer(&song->parent, Song_set_buffer_size);
    Device_set_sync(&song->parent, Song_sync);
    Device_register_port(&song->parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    song->subsongs = NULL;
    song->pats = NULL;
    song->insts = NULL;
    song->effects = NULL;
    song->connections = NULL;
    song->play_state = NULL;
    song->event_handler = NULL;
    song->skip_state = NULL;
    song->skip_handler = NULL;
    song->random = NULL;
    song->env = NULL;
    song->set_map = NULL;
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
    {
        song->scales[i] = NULL;
    }
    song->random = new_Random();
    song->subsongs = new_Subsong_table();
    song->pats = new_Pat_table(KQT_PATTERNS_MAX);
    song->insts = new_Ins_table(KQT_INSTRUMENTS_MAX);
    song->effects = new_Effect_table(KQT_EFFECTS_MAX);
    if (song->random == NULL       ||
            song->subsongs == NULL ||
            song->pats == NULL     ||
            song->insts == NULL    ||
            song->effects == NULL)
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
    song->env = new_Environment();
    if (song->env == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->play_state = new_Playdata(song->insts,
                                    song->env,
                                    song->random);
    if (song->play_state == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->play_state->subsongs = Song_get_subsongs(song);
    song->play_state->scales = song->scales;
    song->play_state->active_scale = &song->play_state->scales[0];
    song->skip_state = new_Playdata_silent(song->env, 1000000000);
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
    song->connections = new_Connections_from_string(NULL, false,
                                                    song->insts,
                                                    song->effects,
                                                    NULL,
                                                    &song->parent,
                                                    conn_state);
    if (song->connections == NULL)
    {
        assert(!conn_state->error);
        del_Song(song);
        return NULL;
    }
    if (!Connections_prepare(song->connections))
    {
        del_Song(song);
        return NULL;
    }

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        song->channels[i] = new_Channel(song->insts, i,
                                        song->play_state->voice_pool,
                                        song->env,
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
                                            song->insts,
                                            song->effects);
    song->skip_handler = new_Event_handler(song->skip_state,
                                           ch_states,
                                           song->insts,
                                           song->effects);
    if (song->event_handler == NULL || song->skip_handler == NULL)
    {
        del_Song(song);
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    song->set_map = new_Set_map_from_string(NULL,
                            Event_handler_get_names(song->event_handler),
                            state);
    if (song->set_map == NULL)
    {
        assert(!state->error);
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
    Song_set_random_seed(song, 0);
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
                if (string_eq(key, "mix_vol"))
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
                else if (string_eq(key, "init_subsong"))
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
    song->mix_vol_dB = mix_vol;
    song->mix_vol = exp2(song->mix_vol_dB / 6);
    Song_set_subsong(song, init_subsong);
    return true;
}


bool Song_parse_random_seed(Song* song, char* str, Read_state* state)
{
    assert(song != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    int64_t seed = 0;
    if (str != NULL)
    {
        str = read_int(str, &seed, state);
        if (state->error)
        {
            return false;
        }
        if (seed < 0)
        {
            Read_state_set_error(state, "Random seed must be positive");
            return false;
        }
    }
    Song_set_random_seed(song, seed);
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
    if (nframes > Device_get_buffer_size((Device*)song) && !play->silent)
    {
        nframes = Device_get_buffer_size((Device*)song);
    }
    if (!play->silent)
    {
        if (song->connections != NULL)
        {
            Connections_clear_buffers(song->connections, 0, nframes);
        }
        Voice_pool_prepare(play->voice_pool);
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
        if (pat == NULL && !play->parent.pause /*&& play->mode != PLAY_EVENT*/)
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
            Playdata_set_subsong(play, play->orig_subsong + 1);
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
        assert(bufs[0] != NULL);
        assert(bufs[1] != NULL);
        for (int i = 0; i < 2; ++i)
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
    if (!play->parent.pause)
    {
        play->play_frames += mixed;
    }
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
        if (inc < Device_get_buffer_size((Device*)song))
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


Effect_table* Song_get_effects(Song* song)
{
    assert(song != NULL);
    return song->effects;
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


static void Song_reset(Device* device)
{
    assert(device != NULL);
    Song* song = (Song*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(song->insts, i);
        if (ins != NULL)
        {
            Device_reset((Device*)ins);
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(song->effects, i);
        if (eff != NULL)
        {
            Device_reset((Device*)eff);
        }
    }
    Event_handler_clear_buffers(song->event_handler);
    Playdata_reset(song->play_state);
    Playdata_reset(song->skip_state);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_reset(song->channels[i]);
    }
    Random_reset(song->random);
    return;
}


static void Song_set_random_seed(Song* song, uint64_t seed)
{
    assert(song != NULL);
    song->random_seed = seed;
    Random_set_seed(song->random, seed);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_set_random_seed(song->channels[i], seed);
    }
    return;
}


static bool Song_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    Song* song = (Song*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(song->insts, i);
        if (ins != NULL && !Device_set_mix_rate((Device*)ins, mix_rate))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(song->effects, i);
        if (eff != NULL && !Device_set_mix_rate((Device*)eff, mix_rate))
        {
            return false;
        }
    }
    return true;
}


static bool Song_set_buffer_size(Device* device, uint32_t size)
{
    assert(device != NULL);
    Song* song = (Song*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(song->insts, i);
        if (ins != NULL && !Device_set_buffer_size((Device*)ins, size))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(song->effects, i);
        if (eff != NULL && !Device_set_buffer_size((Device*)eff, size))
        {
            return false;
        }
    }
    return true;
}


static bool Song_sync(Device* device)
{
    assert(device != NULL);
    Song* song = (Song*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(song->insts, i);
        if (ins != NULL && !Device_sync((Device*)ins))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(song->effects, i);
        if (eff != NULL && !Device_sync((Device*)eff))
        {
            return false;
        }
    }
    return true;
}


void del_Song(Song* song)
{
    if (song == NULL)
    {
        return;
    }
    del_Environment(song->env);
    del_Subsong_table(song->subsongs);
    del_Pat_table(song->pats);
    del_Connections(song->connections);
    del_Ins_table(song->insts);
    del_Effect_table(song->effects);
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
    {
        del_Scale(song->scales[i]);
    }
    del_Playdata(song->play_state);
    del_Playdata(song->skip_state);
    for (int i = 0; i < KQT_COLUMNS_MAX && song->channels[i] != NULL; ++i)
    {
        del_Channel(song->channels[i]);
    }
    del_Event_handler(song->event_handler);
    del_Event_handler(song->skip_handler);
    del_Random(song->random);
    del_Set_map(song->set_map);
    Device_uninit(&song->parent);
    xfree(song);
    return;
}


