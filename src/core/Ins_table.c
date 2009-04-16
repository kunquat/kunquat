

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


