

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

#include <Subsong.h>
#include <Etable.h>
#include <Subsong_table.h>

#include <xmemory.h>


struct Subsong_table
{
    Etable* subs;
};


Subsong_table* new_Subsong_table(void)
{
    Subsong_table* table = xalloc(Subsong_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->subs = new_Etable(KQT_SUBSONGS_MAX, (void(*)(void*))del_Subsong);
    if (table->subs == NULL)
    {
        xfree(table);
        return NULL;
    }
    return table;
}


bool Subsong_table_read(Subsong_table* table, File_tree* tree, Read_state* state)
{
    assert(table != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Subsong collection is not a directory");
        return false;
    }
    for (int i = 0; i < KQT_SUBSONGS_MAX; ++i)
    {
        char dir_name[] = "subsong_xx";
        snprintf(dir_name, 11, "subsong_%02x", i);
        File_tree* subsong_tree = File_tree_get_child(tree, dir_name);
        if (subsong_tree != NULL)
        {
            Read_state_init(state, File_tree_get_path(tree));
            Subsong* ss = new_Subsong();
            if (ss == NULL)
            {
                Read_state_set_error(state,
                         "Couldn't allocate memory for subsong %02x", i);
                return false;
            }
            Subsong_read(ss, subsong_tree, state);
            if (state->error)
            {
                return false;
            }
            if (Subsong_table_set(table, i, ss) < 0)
            {
                Read_state_set_error(state,
                         "Couldn't allocate memory for subsong %02x", i);
                return false;
            }
        }
        else
        {
            break;
        }
    }
    return true;
}


int16_t Subsong_table_set(Subsong_table* table, uint16_t index, Subsong* subsong)
{
    assert(table != NULL);
    assert(index < KQT_SUBSONGS_MAX);
    assert(subsong != NULL);
    while (index > 0 && Etable_get(table->subs, index - 1) == NULL)
    {
        --index;
    }
    if (!Etable_set(table->subs, index, subsong))
    {
        return -1;
    }
    return index;
}


Subsong* Subsong_table_get(Subsong_table* table, uint16_t index)
{
    assert(table != NULL);
    assert(index < KQT_SUBSONGS_MAX);
    return Etable_get(table->subs, index);
}


bool Subsong_table_is_empty(Subsong_table* table, uint16_t subsong)
{
    assert(table != NULL);
    assert(subsong < KQT_SUBSONGS_MAX);
    Subsong* ss = Etable_get(table->subs, subsong);
    if (ss == NULL)
    {
        return true;
    }
    for (int i = 0; i < ss->res; ++i)
    {
        if (ss->pats[i] != KQT_SECTION_NONE)
        {
            return false;
        }
    }
    return true;
}


void del_Subsong_table(Subsong_table* table)
{
    assert(table != NULL);
    del_Etable(table->subs);
    xfree(table);
    return;
}


