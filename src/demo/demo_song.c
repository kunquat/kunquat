

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
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    File_tree* song_tree = new_File_tree_from_fs("kunquat_c_00", state);
//    File_tree* song_tree = new_File_tree_from_tar("demo.kqt.bz2", state);
    if (song_tree == NULL)
    {
        del_Song(song);
        fprintf(stderr, "Error while loading the demo song: %s\n", state->message);
        return NULL;
    }
    if (!Song_read(song, song_tree, state))
    {
        del_File_tree(song_tree);
        del_Song(song);
        fprintf(stderr, "Error while loading the demo song:\n   %s:%d: %s\n",
                        state->path, state->row, state->message);
        return NULL;
    }
    del_File_tree(song_tree);

    return song;
}


