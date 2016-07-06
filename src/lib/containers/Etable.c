

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <containers/Etable.h>

#include <debug/assert.h>
#include <memory.h>

#include <stdlib.h>


struct Etable
{
    int size;
    int res;
    void** els;
    void (*destroy)(void* el);
};


Etable* new_Etable(int size, void (*destroy)(void*))
{
    rassert(size > 0);

    Etable* table = memory_alloc_item(Etable);
    if (table == NULL)
        return NULL;

    table->size = size;
    table->res = 8;
    if (size < 8)
        table->res = size;

    table->els = memory_alloc_items(void*, table->res);
    if (table->els == NULL)
    {
        memory_free(table);
        return NULL;
    }

    for (int i = 0; i < table->res; ++i)
        table->els[i] = NULL;

    table->destroy = destroy;

    return table;
}


bool Etable_set(Etable* table, int index, void* el)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);
    rassert(el != NULL);

#ifndef NDEBUG
    for (int i = 0; i < table->res; ++i)
        rassert(table->els[i] != el);
#endif

    if (index >= table->res)
    {
        int new_res = table->res << 1;
        if (index >= new_res)
            new_res = index + 1;

        void** new_els = memory_realloc_items(void*, new_res, table->els);
        if (new_els == NULL)
            return false;

        table->els = new_els;
        for (int i = table->res; i < new_res; ++i)
            table->els[i] = NULL;

        table->res = new_res;
    }

    if (table->els[index] != NULL)
        table->destroy(table->els[index]);

    table->els[index] = el;

    return true;
}


void* Etable_get(Etable* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    if (index >= table->res)
        return NULL;

    return table->els[index];
}


void Etable_remove(Etable* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    if (index >= table->res || table->els[index] == NULL)
        return;

    table->destroy(table->els[index]);
    table->els[index] = NULL;

    return;
}


void Etable_clear(Etable* table)
{
    rassert(table != NULL);

    for (int i = 0; i < table->res; ++i)
    {
        table->destroy(table->els[i]);
        table->els[i] = NULL;
    }

    return;
}


void del_Etable(Etable* table)
{
    if (table == NULL)
        return;

    Etable_clear(table);
    memory_free(table->els);
    memory_free(table);

    return;
}


