

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

#include <Etable.h>
#include <Instrument.h>
#include <Ins_table.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


struct Ins_table
{
    int size;
    Etable* insts;
};


Ins_table* new_Ins_table(int size)
{
    assert(size > 0);
    Ins_table* table = xalloc(Ins_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->insts = new_Etable(size, (void (*)(void*))del_Instrument);
    if (table->insts == NULL)
    {
        xfree(table);
        return NULL;
    }
    table->size = size;
    return table;
}


bool Ins_table_read(Ins_table* table, File_tree* tree, Read_state* state,
                    frame_t** bufs,
                    frame_t** voice_bufs,
                    int buf_count,
                    uint32_t buf_len,
                    Note_table** note_tables,
                    Note_table** default_notes,
                    uint8_t events)
{
    assert(table != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    assert(bufs != NULL);
    assert(voice_bufs != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
    assert(note_tables != NULL);
    assert(events > 0);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Instrument table is not a directory");
        return false;
    }
    for (int i = 1; i < INSTRUMENTS_MAX; ++i)
    {
        char dir_name[] = "i_00";
        snprintf(dir_name, 5, "i_%02x", i);
        File_tree* index_tree = File_tree_get_child(tree, dir_name);
        if (index_tree != NULL)
        {
            Read_state_init(state, File_tree_get_path(index_tree));
            if (!File_tree_is_dir(index_tree))
            {
                Read_state_set_error(state,
                         "Instrument index %02x is not a directory", i);
                return false;
            }
            File_tree* ins_tree = File_tree_get_child(index_tree, "kunquati00");
            if (ins_tree != NULL)
            {
                Instrument* ins = new_Instrument(bufs,
                                                 voice_bufs,
                                                 buf_count,
                                                 buf_len,
                                                 note_tables,
                                                 default_notes,
                                                 events);
                if (ins == NULL)
                {
                    Read_state_set_error(state,
                             "Couldn't allocate memory for instrument %02x", i);
                    return false;
                }
                Instrument_read(ins, ins_tree, state);
                if (state->error)
                {
                    del_Instrument(ins);
                    return false;
                }
                Read_state_init(state, File_tree_get_path(ins_tree));
                if (!Ins_table_set(table, i, ins))
                {
                    Read_state_set_error(state,
                             "Couldn't insert Instrument %02x into the Instrument table", i);
                    return false;
                }
            }
        }
    }
    return true;
}


bool Ins_table_set(Ins_table* table, int index, Instrument* ins)
{
    assert(table != NULL);
    assert(index > 0);
    assert(index <= table->size);
    assert(ins != NULL);
    --index;
    return Etable_set(table->insts, index, ins);
}


Instrument* Ins_table_get(Ins_table* table, int index)
{
    assert(table != NULL);
    assert(index > 0);
    assert(index <= table->size);
    --index;
    return Etable_get(table->insts, index);
}


void Ins_table_remove(Ins_table* table, int index)
{
    assert(table != NULL);
    assert(index > 0);
    assert(index <= table->size);
    --index;
    Etable_remove(table->insts, index);
    return;
}


void Ins_table_clear(Ins_table* table)
{
    assert(table != NULL);
    Etable_clear(table->insts);
    return;
}


void del_Ins_table(Ins_table* table)
{
    assert(table != NULL);
    del_Etable(table->insts);
    xfree(table);
    return;
}


