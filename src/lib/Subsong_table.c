

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Bit_array.h>
#include <Etable.h>
#include <memory.h>
#include <Subsong.h>
#include <Subsong_table.h>
#include <xassert.h>


struct Subsong_table
{
    int effective_size;
    Etable* subs;
    Bit_array* existents;
};


Subsong_table* new_Subsong_table(void)
{
    Subsong_table* table = memory_alloc_item(Subsong_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->subs = NULL;
    table->existents = NULL;

    table->effective_size = 0;
    table->subs = new_Etable(KQT_SONGS_MAX, (void(*)(void*))del_Subsong);
    table->existents = new_Bit_array(KQT_SONGS_MAX);
    if (table->subs == NULL || table->existents == NULL)
    {
        del_Subsong_table(table);
        return NULL;
    }
    return table;
}


bool Subsong_table_set(Subsong_table* table, uint16_t index, Subsong* subsong)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);
    assert(subsong != NULL);
    if (!Etable_set(table->subs, index, subsong))
    {
        return false;
    }
    if (index == table->effective_size)
    {
        while (index < KQT_SONGS_MAX &&
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
    assert(index < KQT_SONGS_MAX);
    if (index >= table->effective_size)
    {
        return NULL;
    }
    return Etable_get(table->subs, index);
}


void Subsong_table_set_existent(
        Subsong_table* table,
        uint16_t index,
        bool existent)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);

    Bit_array_set(table->existents, index, existent);

    return;
}


bool Subsong_table_get_existent(Subsong_table* table, uint16_t index)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);

    return Bit_array_get(table->existents, index);
}


#if 0
Subsong* Subsong_table_get_hidden(Subsong_table* table, uint16_t index)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);
    return Etable_get(table->subs, index);
}
#endif


bool Subsong_table_is_empty(Subsong_table* table, uint16_t subsong)
{
    assert(table != NULL);
    assert(subsong < KQT_SONGS_MAX);
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
    if (table == NULL)
    {
        return;
    }
    del_Etable(table->subs);
    del_Bit_array(table->existents);
    memory_free(table);
    return;
}


