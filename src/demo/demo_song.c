

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

#include "demo_song.h"

#include <Generator_sine.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>
#include <File_base.h>


Song* demo_song_create(void)
{
    FILE* in = NULL;
    Song* song = new_Song(2, 128, 16);
    if (song == NULL)
    {
        goto cleanup;
    }
    Song_set_name(song, L"Demo");
    Note_table* notes = Song_get_notes(song, 0);
    Note_table_set_ref_pitch(notes, 528);
    int nums[] = { 16, 9, 6, 5, 4, 45, 3, 8, 5, 9, 15 };
    int dens[] = { 15, 8, 5, 4, 3, 32, 2, 5, 3, 5, 8 };
    for (int i = 1; i <= 11; ++i)
    {
        Note_table_set_note(notes, i,
                Note_table_get_note_name(notes, i),
                Real_init_as_frac(REAL_AUTO, nums[i - 1], dens[i - 1]));
    }
    Note_table_set_name(notes, L"5-limit JI (C major)");
    Song_set_tempo(song, 0, 110);
    Song_set_global_vol(song, 0, 0);
    frame_t** bufs = Song_get_bufs(song);
    Instrument* ins = new_Instrument(bufs,
                                     Song_get_voice_bufs(song),
                                     Song_get_buf_count(song),
                                     Song_get_buf_size(song),
                                     16);
    if (ins == NULL)
    {
        goto cleanup;
    }
    Generator_sine* sine_gen = new_Generator_sine(Instrument_get_params(ins));
    if (sine_gen == NULL)
    {
        del_Instrument(ins);
        goto cleanup;
    }
    Instrument_set_gen(ins, 0, (Generator*)sine_gen);
    Instrument_set_name(ins, L"sine");
    Instrument_set_note_table(ins, Song_get_active_notes(song));
    Ins_table* insts = Song_get_insts(song);
    if (!Ins_table_set(insts, 1, ins))
    {
        del_Instrument(ins);
        goto cleanup;
    }
    
    Pattern* pat = new_Pattern();
    if (pat == NULL)
    {
        goto cleanup;
    }
    Pat_table* pats = Song_get_pats(song);
    if (!Pat_table_set(pats, 0, pat))
    {
        del_Pattern(pat);
        goto cleanup;
    }
    Pattern_set_length(pat, Reltime_set(RELTIME_AUTO, 8, 0));
    in = fopen("kunquat_m_00/patterns/000/000/events.json", "rb");
    if (in == NULL)
    {
        goto cleanup;
    }
    Read_state* state = &(Read_state){ .error = false, .message = { '\0' }, .row = 0 };
    if (!Column_read(Pattern_get_col(pat, 0), in, state))
    {
        fprintf(stderr, "Read error @ row %d: %s\n", state->row, state->message);
        goto cleanup;
    }
    fclose(in);
    in = NULL;
    
    pat = new_Pattern();
    if (pat == NULL)
    {
        goto cleanup;
    }
    if (!Pat_table_set(pats, 1, pat))
    {
        del_Pattern(pat);
        goto cleanup;
    }
    Pattern_set_length(pat, Reltime_set(RELTIME_AUTO, 8, 0));
    in = fopen("kunquat_m_00/patterns/001/000/events.json", "rb");
    if (in == NULL)
    {
        goto cleanup;
    }
    state->row = 0;
    if (!Column_read(Pattern_get_col(pat, 0), in, state))
    {
        fprintf(stderr, "Read error @ row %d: %s\n", state->row, state->message);
        goto cleanup;
    }
    fclose(in);
    in = NULL;
    in = fopen("kunquat_m_00/patterns/001/001/events.json", "rb");
    if (in == NULL)
    {
        goto cleanup;
    }
    state->row = 0;
    if (!Column_read(Pattern_get_col(pat, 1), in, state))
    {
        fprintf(stderr, "Read error @ row %d: %s\n", state->row, state->message);
        goto cleanup;
    }
    fclose(in);
    in = NULL;

    Order* order = Song_get_order(song);
    if (!Order_set(order, 0, 0, 0))
    {
        goto cleanup;
    }
    if (!Order_set(order, 0, 1, 1))
    {
        goto cleanup;
    }
    if (!Order_set(order, 0, 2, 0))
    {
        goto cleanup;
    }
    return song;

cleanup:

    if (song != NULL)
    {
        del_Song(song);
    }
    if (in != NULL)
    {
        fclose(in);
    }
    return NULL;
}


