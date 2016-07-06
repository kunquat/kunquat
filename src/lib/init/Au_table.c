

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


#include <init/Au_table.h>

#include <containers/Etable.h>
#include <debug/assert.h>
#include <init/devices/Audio_unit.h>
#include <memory.h>

#include <stdbool.h>
#include <stdlib.h>


struct Au_table
{
    int size;
    Etable* aus;
};


Au_table* new_Au_table(int size)
{
    rassert(size > 0);

    Au_table* table = memory_alloc_item(Au_table);
    if (table == NULL)
        return NULL;

    table->aus = new_Etable(size, (void (*)(void*))del_Audio_unit);
    if (table->aus == NULL)
    {
        memory_free(table);
        return NULL;
    }

    table->size = size;

    return table;
}


bool Au_table_set(Au_table* table, int index, Audio_unit* au)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);
    rassert(au != NULL);

    return Etable_set(table->aus, index, au);
}


Audio_unit* Au_table_get(Au_table* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    return Etable_get(table->aus, index);
}


void Au_table_remove(Au_table* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    Etable_remove(table->aus, index);

    return;
}


void Au_table_clear(Au_table* table)
{
    rassert(table != NULL);
    Etable_clear(table->aus);
    return;
}


void del_Au_table(Au_table* table)
{
    if (table == NULL)
        return;

    del_Etable(table->aus);
    memory_free(table);

    return;
}


