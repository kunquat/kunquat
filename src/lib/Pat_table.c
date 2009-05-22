

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

#include <Pat_table.h>

#include <Pattern.h>

#include <xmemory.h>


Pat_table* new_Pat_table(int size)
{
    assert(size > 0);
    Pat_table* table = xalloc(Pat_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->pats = new_Etable(size, (void (*)(void*))del_Pattern);
    if (table->pats == NULL)
    {
        xfree(table);
        return NULL;
    }
    table->size = size;
    return table;
}


bool Pat_table_set(Pat_table* table, int index, Pattern* pat)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    assert(pat != NULL);
    return Etable_set(table->pats, index, pat);
}


Pattern* Pat_table_get(Pat_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    return Etable_get(table->pats, index);
}


void Pat_table_remove(Pat_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    Etable_remove(table->pats, index);
    return;
}


void Pat_table_clear(Pat_table* table)
{
    assert(table != NULL);
    Etable_clear(table->pats);
    return;
}


void del_Pat_table(Pat_table* table)
{
    assert(table != NULL);
    del_Etable(table->pats);
    xfree(table);
    return;
}


