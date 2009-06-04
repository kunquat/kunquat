

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
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>

#include <Real.h>

#include <Song.h>

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
    song->mix_vol = -8;
    song->init_subsong = 0;
    return song;
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
    double vol = exp2(song->mix_vol / 6);
    for (int i = 0; i < song->buf_count; ++i)
    {
        for (uint32_t k = 0; k < mixed; ++k)
        {
            song->bufs[i][k] *= vol;
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
    song->mix_vol = mix_vol;
    return;
}


double Song_get_mix_vol(Song* song)
{
    assert(song != NULL);
    return song->mix_vol;
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


