

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
#include <File_tree.h>


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

    File_tree* pats_tree = new_File_tree_from_fs("kunquat_m_00/patterns");
    if (pats_tree == NULL)
    {
        goto cleanup;
    }
    Read_state* state = &(Read_state){ .error = false, .message = { '\0' }, .row = 0 };
    Pat_table* pats = Song_get_pats(song);
    if (!Pat_table_read(pats, pats_tree, state))
    {
        del_File_tree(pats_tree);
        goto cleanup;
    }
    del_File_tree(pats_tree);

    File_tree* order_tree = new_File_tree_from_fs("kunquat_m_00/subsongs");
    if (order_tree == NULL)
    {
        goto cleanup;
    }
    Order* order = Song_get_order(song);
    if (!Order_read(order, order_tree, state))
    {
        del_File_tree(order_tree);
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


