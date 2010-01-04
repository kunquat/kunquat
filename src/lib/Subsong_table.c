

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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
    int effective_size;
    Etable* subs;
};


Subsong_table* new_Subsong_table(void)
{
    Subsong_table* table = xalloc(Subsong_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->effective_size = 0;
    table->subs = new_Etable(KQT_SUBSONGS_MAX, (void(*)(void*))del_Subsong);
    if (table->subs == NULL)
    {
        xfree(table);
        return NULL;
    }
    return table;
}


bool Subsong_table_set(Subsong_table* table, uint16_t index, Subsong* subsong)
{
    assert(table != NULL);
    assert(index < KQT_SUBSONGS_MAX);
    assert(subsong != NULL);
    if (!Etable_set(table->subs, index, subsong))
    {
        return false;
    }
    if (index == table->effective_size)
    {
        while (index < KQT_SUBSONGS_MAX &&
                Etable_get(table->subs, index) != NULL)
        {
            table->effective_size = index + 1;
            ++index;
        }
    }
    return true;
}


Subsong* Subsong_table_get(Subsong_table* table, uint16_t index)
{
    assert(table != NULL);
    assert(index < KQT_SUBSONGS_MAX);
    if (index >= table->effective_size)
    {
        return NULL;
    }
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


