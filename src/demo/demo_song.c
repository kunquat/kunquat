

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
    Song* song = new_Song(2, 128, 16);
    if (song == NULL)
    {
        goto cleanup;
    }
    Song_set_name(song, L"Demo");

    Read_state* state = &(Read_state){ .error = false, .message = { '\0' }, .row = 1 };
    File_tree* notes_tree = new_File_tree_from_fs("kunquat_m_00/tunings/0/kunquat_t_00");
    if (notes_tree == NULL)
    {
        goto cleanup;
    }
    Note_table* notes = Song_get_notes(song, 0);
    if (!Note_table_read(notes, notes_tree, state))
    {
        fprintf(stderr, "Note table reading failed @ row %d: %s\n", state->row, state->message);
        del_File_tree(notes_tree);
        goto cleanup;
    }
    del_File_tree(notes_tree);

    Ins_table* insts = Song_get_insts(song);
    File_tree* insts_tree = new_File_tree_from_fs("kunquat_m_00/instruments");
    if (insts_tree == NULL)
    {
        goto cleanup;
    }
    if (!Ins_table_read(insts, insts_tree, state,
                        Song_get_bufs(song),
                        Song_get_voice_bufs(song),
                        Song_get_buf_count(song),
                        Song_get_buf_size(song),
                        Song_get_note_tables(song),
                        Song_get_active_notes(song),
                        16))
    {
        del_File_tree(insts_tree);
        goto cleanup;
    }
    del_File_tree(insts_tree);

    File_tree* pats_tree = new_File_tree_from_fs("kunquat_m_00/patterns");
    if (pats_tree == NULL)
    {
        goto cleanup;
    }
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
    del_File_tree(order_tree);

    return song;

cleanup:

    if (song != NULL)
    {
        del_Song(song);
    }
    return NULL;
}


