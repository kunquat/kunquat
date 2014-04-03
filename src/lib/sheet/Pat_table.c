

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>

#include <Bit_array.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <sheet/Pat_table.h>
#include <sheet/Pattern.h>
#include <xassert.h>


struct Pat_table
{
    int size;
    Etable* pats;
    Bit_array* existents;
};


Pat_table* new_Pat_table(int size)
{
    assert(size > 0);

    Pat_table* table = memory_alloc_item(Pat_table);
    if (table == NULL)
        return NULL;

    table->pats = NULL;
    table->existents = NULL;

    table->pats = new_Etable(size, (void (*)(void*))del_Pattern);
    table->existents = new_Bit_array(size);
    if (table->pats == NULL || table->existents == NULL)
    {
        del_Pat_table(table);
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


void Pat_table_set_existent(Pat_table* table, int index, bool existent)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    Bit_array_set(table->existents, index, existent);

    return;
}


bool Pat_table_get_existent(const Pat_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    return Bit_array_get(table->existents, index);
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
    if (table == NULL)
        return;

    del_Etable(table->pats);
    del_Bit_array(table->existents);
    memory_free(table);

    return;
}


