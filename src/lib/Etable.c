

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

#include <xmemory.h>


struct Etable
{
    int size;
    int res;
    void** els;
    void (*destroy)(void* el);
};


Etable* new_Etable(int size, void (*destroy)(void*))
{
    assert(size > 0);
    Etable* table = xalloc(Etable);
    if (table == NULL)
    {
        return NULL;
    }
    table->size = size;
    table->res = 8;
    if (size < 8)
    {
        table->res = size;
    }
    table->els = xnalloc(void*, table->res);
    if (table->els == NULL)
    {
        xfree(table);
        return NULL;
    }
    for (int i = 0; i < table->res; ++i)
    {
        table->els[i] = NULL;
    }
    table->destroy = destroy;
    return table;
}


bool Etable_set(Etable* table, int index, void* el)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    assert(el != NULL);
#ifndef NDEBUG
    for (int i = 0; i < table->res; ++i)
    {
        assert(table->els[i] != el);
    }
#endif
    if (index >= table->res)
    {
        int new_res = table->res << 1;
        if (index >= new_res)
        {
            new_res = index + 1;
        }
        void** new_els = xrealloc(void*, new_res, table->els);
        if (new_els == NULL)
        {
            return false;
        }
        table->els = new_els;
        for (int i = table->res; i < new_res; ++i)
        {
            table->els[i] = NULL;
        }
        table->res = new_res;
    }
    if (table->els[index] != NULL)
    {
        table->destroy(table->els[index]);
    }
    table->els[index] = el;
    return true;
}


void* Etable_get(Etable* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    if (index >= table->res)
    {
        return NULL;
    }
    return table->els[index];
}


void Etable_remove(Etable* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    if (index >= table->res || table->els[index] == NULL)
    {
        return;
    }
    table->destroy(table->els[index]);
    table->els[index] = NULL;
    return;
}


void Etable_clear(Etable* table)
{
    assert(table != NULL);
    for (int i = 0; i < table->res; ++i)
    {
        if (table->els[i] != NULL)
        {
            table->destroy(table->els[i]);
            table->els[i] = NULL;
        }
    }
    return;
}


void del_Etable(Etable* table)
{
    assert(table != NULL);
    Etable_clear(table);
    xfree(table->els);
    xfree(table);
    return;
}


