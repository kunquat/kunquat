

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>

#include <Real.h>
#include <Song.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


Song* new_Song(int buf_count, uint32_t buf_size, uint8_t events)
{
    assert(buf_count >= 1);
    assert(buf_count <= BUF_COUNT_MAX);
    assert(buf_size > 0);
    Song* song = xalloc(Song);
    if (song == NULL)
    {
        return NULL;
    }
    song->buf_count = buf_count;
    song->buf_size = buf_size;
    song->bufs = NULL;
    song->priv_bufs[0] = NULL;
    song->voice_bufs[0] = NULL;
    song->order = NULL;
    song->pats = NULL;
    song->insts = NULL;
    for (int i = 0; i < NOTE_TABLES_MAX; ++i)
    {
        song->notes[i] = NULL;
    }
    song->active_notes = &song->notes[0];
    song->bufs = xnalloc(frame_t*, BUF_COUNT_MAX);
    if (song->bufs == NULL)
    {
        del_Song(song);
        return NULL;
    }
    for (int i = 0; i < buf_count; ++i)
    {
        song->priv_bufs[i] = xnalloc(frame_t, buf_size);
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
        song->voice_bufs[i] = xnalloc(frame_t, buf_size);
        if (song->voice_bufs[i] == NULL)
        {
            del_Song(song);
            return NULL;
        }
    }
    song->order = new_Order();
    if (song->order == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->pats = new_Pat_table(PATTERNS_MAX);
    if (song->pats == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->insts = new_Ins_table(INSTRUMENTS_MAX);
    if (song->insts == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->notes[0] = new_Note_table(523.25113060119725,
            Real_init_as_frac(REAL_AUTO, 2, 1));
    if (song->notes[0] == NULL)
    {
        del_Song(song);
        return NULL;
    }
    song->events = new_Event_queue(events);
    if (song->events == NULL)
    {
        del_Song(song);
        return NULL;
    }
    Note_table_set_note(song->notes[0],
            0,
            Real_init_as_frac(REAL_AUTO, 1, 1));
    for (int i = 1; i < 12; ++i)
    {
        Note_table_set_note_cents(song->notes[0],
                i,
                i * 100);
    }
    song->name[0] = song->name[SONG_NAME_MAX - 1] = L'\0';
    song->mix_vol_dB = -8;
    song->mix_vol = exp2(song->mix_vol_dB / 6);
    song->init_subsong = 0;
    return song;
}


bool Song_read(Song* song, File_tree* tree, Read_state* state)
{
    assert(song != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Song is not a directory");
        return false;
    }
    char* name = File_tree_get_name(tree);
    if (strncmp(name, MAGIC_ID, strlen(MAGIC_ID)) != 0)
    {
        Read_state_set_error(state, "Directory is not a Kunquat file");
        return false;
    }
    if (strncmp(name + strlen(MAGIC_ID), "s_", 2) != 0)
    {
        Read_state_set_error(state, "Directory is not a Song file");
        return false;
    }
    const char* version = "00";
    if (strcmp(name + strlen(MAGIC_ID) + 2, version) != 0)
    {
        Read_state_set_error(state, "Unsupported Song version");
        return false;
    }
    File_tree* info_tree = File_tree_get_child(tree, "info_song.json");
    if (info_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(info_tree));
        if (File_tree_is_dir(info_tree))
        {
            Read_state_set_error(state, "Song info is a directory");
            return false;
        }
        char* str = File_tree_get_data(info_tree);
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
                if (strcmp(key, "buf_count") == 0)
                {
                    int64_t num = 0;
                    str = read_int(str, &num, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (num < 1 || num > BUF_COUNT_MAX)
                    {
                        Read_state_set_error(state,
                                 "Unsupported number of mixing buffers: %" PRId64, num);
                        return false;
                    }
                    if (!Song_set_buf_count(song, num))
                    {
                        Read_state_set_error(state,
                                 "Couldn't allocate memory for mixing buffers");
                        return false;
                    }
                }
                else if (strcmp(key, "mix_vol") == 0)
                {
                    str = read_double(str, &song->mix_vol_dB, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (!isfinite(song->mix_vol_dB) && song->mix_vol_dB != -INFINITY)
                    {
                        Read_state_set_error(state,
                                 "Invalid mixing volume: %f", song->mix_vol_dB);
                        song->mix_vol_dB = 0;
                        return false;
                    }
                    song->mix_vol = exp2(song->mix_vol_dB / 6);
                }
                else if (strcmp(key, "init_subsong") == 0)
                {
                    int64_t num = 0;
                    str = read_int(str, &num, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if (num < 0 || num >= SUBSONGS_MAX)
                    {
                        Read_state_set_error(state,
                                 "Invalid initial Subsong number: %" PRId64, num);
                        return false;
                    }
                    Song_set_subsong(song, num);
                }
                else
                {
                    Read_state_set_error(state,
                             "Unrecognised key in Song info: %s", key);
                    return false;
                }
                if (state->error)
                {
                    return false;
                }
                str = read_const_char(str, ',', state);
                if (state->error)
                {
                    expect_key = false;
                    Read_state_clear_error(state);
                }
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    File_tree* nts_tree = File_tree_get_child(tree, "tunings");
    if (nts_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(nts_tree));
        if (!File_tree_is_dir(nts_tree))
        {
            Read_state_set_error(state,
                     "Note table collection is not a directory");
            return false;
        }
        for (int i = 0; i < NOTE_TABLES_MAX; ++i)
        {
            char dir_name[] = "t_0";
            snprintf(dir_name, 4, "t_%01x", i);
            File_tree* index_tree = File_tree_get_child(nts_tree, dir_name);
            if (index_tree != NULL)
            {
                Read_state_init(state, File_tree_get_path(index_tree));
                if (!File_tree_is_dir(index_tree))
                {
                    Read_state_set_error(state,
                             "Note table at index %01x is not a directory", i);
                    return false;
                }
                File_tree* notes_tree = File_tree_get_child(index_tree, "kunquat_t_00");
                if (notes_tree != NULL)
                {
                    Read_state_init(state, File_tree_get_path(notes_tree));
                    if (!Song_create_notes(song, i))
                    {
                        Read_state_set_error(state,
                                 "Couldn't allocate memory for Note table %01x", i);
                        return false;
                    }
                    Note_table* notes = Song_get_notes(song, i);
                    assert(notes != NULL);
                    Note_table_read(notes, notes_tree, state);
                    if (state->error)
                    {
                        Song_remove_notes(song, i);
                        return false;
                    }
                }
            }
        }
    }
    File_tree* ss_tree = File_tree_get_child(tree, "subsongs");
    if (ss_tree != NULL)
    {
        Order* order = Song_get_order(song);
        if (!Order_read(order, ss_tree, state))
        {
            return false;
        }
    }
    File_tree* pat_tree = File_tree_get_child(tree, "patterns");
    if (pat_tree != NULL)
    {
        Pat_table* pats = Song_get_pats(song);
        if (!Pat_table_read(pats, pat_tree, state))
        {
            return false;
        }
    }
    File_tree* ins_tree = File_tree_get_child(tree, "instruments");
    if (ins_tree != NULL)
    {
        Ins_table* insts = Song_get_insts(song);
        if (!Ins_table_read(insts, ins_tree, state,
                            Song_get_bufs(song),
                            Song_get_voice_bufs(song),
                            Song_get_buf_count(song),
                            Song_get_buf_size(song),
                            Song_get_note_tables(song),
                            Song_get_active_notes(song),
                            16)) // TODO: make configurable
        {
            return false;
        }
    }
    return true;
}


uint32_t Song_mix(Song* song, uint32_t nframes, Playdata* play)
{
    assert(song != NULL);
    assert(play != NULL);
    if (play->mode == STOP)
    {
        return 0;
    }
    if (nframes > song->buf_size)
    {
        nframes = song->buf_size;
    }
    for (int i = 0; i < song->buf_count; ++i)
    {
        for (uint32_t k = 0; k < nframes; ++k)
        {
            song->bufs[i][k] = 0;
        }
    }
    uint32_t mixed = 0;
    while (mixed < nframes && play->mode)
    {
        Pattern* pat = NULL;
        if (play->mode == PLAY_SONG)
        {
            int16_t pat_index = Order_get(song->order,
                    play->subsong,
                    play->order_index);
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
            // TODO: Stop or restart?
            play->mode = STOP;
            break;
        }
        uint32_t proc_start = mixed;
        mixed += Pattern_mix(pat, nframes, mixed, play);
        if (play->mode == PLAY_EVENT)
        {
            continue;
        }
        Event* event = NULL;
        uint32_t proc_until = mixed;
        Event_queue_get(song->events, &event, &proc_until);
        // TODO: mix private instrument buffers
        while (proc_start < mixed)
        {
            assert(proc_until <= mixed);
            for (uint32_t i = proc_start; i < proc_until; ++i)
            {
                // TODO: modify buffer according to state
            }
            proc_start = proc_until;
            proc_until = mixed;
            while (Event_queue_get(song->events, &event, &proc_until))
            {
                // TODO: process events
                if (proc_start < mixed)
                {
                    break;
                }
            }
        }
        assert(!Event_queue_get(song->events, &event, &proc_until));
    }
    for (int i = 0; i < song->buf_count; ++i)
    {
        for (uint32_t k = 0; k < mixed; ++k)
        {
            song->bufs[i][k] *= song->mix_vol;
        }
    }
    return mixed;
}


void Song_set_name(Song* song, wchar_t* name)
{
    assert(song != NULL);
    assert(name != NULL);
    wcsncpy(song->name, name, SONG_NAME_MAX - 1);
    song->name[SONG_NAME_MAX - 1] = L'\0';
    return;
}


wchar_t* Song_get_name(Song* song)
{
    assert(song != NULL);
    return song->name;
}


#if 0
void Song_set_tempo(Song* song, int subsong, double tempo)
{
    assert(song != NULL);
    assert(subsong >= 0);
    assert(subsong < SUBSONGS_MAX);
    assert(isfinite(tempo));
    assert(tempo > 0);
    song->subsong_inits[subsong].tempo = tempo;
    return;
}


double Song_get_tempo(Song* song, int subsong)
{
    assert(song != NULL);
    assert(subsong >= 0);
    assert(subsong < SUBSONGS_MAX);
    return song->subsong_inits[subsong].tempo;
}
#endif


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


#if 0
void Song_set_global_vol(Song* song, int subsong, double global_vol)
{
    assert(song != NULL);
    assert(subsong >= 0);
    assert(subsong < SUBSONGS_MAX);
    assert(isfinite(global_vol) || global_vol == -INFINITY);
    song->subsong_inits[subsong].global_vol = global_vol;
    return;
}


double Song_get_global_vol(Song* song, int subsong)
{
    assert(song != NULL);
    assert(subsong >= 0);
    assert(subsong < SUBSONGS_MAX);
    return song->subsong_inits[subsong].global_vol;
}
#endif


void Song_set_subsong(Song* song, uint16_t num)
{
    assert(song != NULL);
    assert(num < SUBSONGS_MAX);
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
    assert(count <= BUF_COUNT_MAX);
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
        }
        // TODO: remove Instrument buffers
    }
    else
    {
        for (int i = song->buf_count; i < count; ++i)
        {
            song->priv_bufs[i] = xnalloc(frame_t, song->buf_size);
            if (song->priv_bufs[i] == NULL)
            {
                return false;
            }
            song->bufs[i] = song->priv_bufs[i];
            song->voice_bufs[i] = xnalloc(frame_t, song->buf_size);
            if (song->voice_bufs[i] == NULL)
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
        frame_t* new_buf = xrealloc(frame_t, size, song->priv_bufs[i]);
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
        new_buf = xrealloc(frame_t, size, song->voice_bufs[i]);
        if (new_buf == NULL)
        {
            if (size < song->buf_size)
            {
                song->buf_size = size;
            }
            return false;
        }
        song->voice_bufs[i] = new_buf;
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


frame_t** Song_get_bufs(Song* song)
{
    assert(song != NULL);
    return song->bufs;
}


frame_t** Song_get_voice_bufs(Song* song)
{
    assert(song != NULL);
    return song->voice_bufs;
}


Order* Song_get_order(Song* song)
{
    assert(song != NULL);
    return song->order;
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


Note_table** Song_get_note_tables(Song* song)
{
    assert(song != NULL);
    return song->notes;
}


Note_table* Song_get_notes(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLES_MAX);
    return song->notes[index];
}


Note_table** Song_get_active_notes(Song* song)
{
    assert(song != NULL);
    return song->active_notes;
}


bool Song_create_notes(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLES_MAX);
    if (song->notes[index] != NULL)
    {
        Note_table_clear(song->notes[index]);
        return true;
    }
    song->notes[index] = new_Note_table(440,
            Real_init_as_frac(REAL_AUTO, 2, 1));
    if (song->notes[index] == NULL)
    {
        return false;
    }
    return true;
}


void Song_remove_notes(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < NOTE_TABLES_MAX);
    if (song->notes[index] != NULL)
    {
        del_Note_table(song->notes[index]);
        song->notes[index] = NULL;
    }
    return;
}


Event_queue* Song_get_events(Song* song)
{
    assert(song != NULL);
    return song->events;
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
    if (song->bufs != NULL)
    {
        xfree(song->bufs);
    }
    if (song->order != NULL)
    {
        del_Order(song->order);
    }
    if (song->pats != NULL)
    {
        del_Pat_table(song->pats);
    }
    if (song->insts != NULL)
    {
        del_Ins_table(song->insts);
    }
    for (int i = 0; i < NOTE_TABLES_MAX; ++i)
    {
        if (song->notes[i] != NULL)
        {
            del_Note_table(song->notes[i]);
        }
    }
    if (song->events != NULL)
    {
        del_Event_queue(song->events);
    }
    xfree(song);
    return;
}


