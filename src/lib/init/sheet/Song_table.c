

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


#include <init/sheet/Song_table.h>

#include <containers/Bit_array.h>
#include <containers/Etable.h>
#include <debug/assert.h>
#include <init/sheet/Song.h>
#include <memory.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Song_table
{
    int effective_size;
    Etable* songs;
    Bit_array* existents;
};


Song_table* new_Song_table(void)
{
    Song_table* table = memory_alloc_item(Song_table);
    if (table == NULL)
        return NULL;

    table->songs = NULL;
    table->existents = NULL;

    table->effective_size = 0;
    table->songs = new_Etable(KQT_SONGS_MAX, (void(*)(void*))del_Song);
    table->existents = new_Bit_array(KQT_SONGS_MAX);
    if (table->songs == NULL || table->existents == NULL)
    {
        del_Song_table(table);
        return NULL;
    }

    return table;
}


bool Song_table_set(Song_table* table, int index, Song* song)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);
    assert(song != NULL);

    if (!Etable_set(table->songs, index, song))
        return false;

    if (index == table->effective_size)
    {
        while (index < KQT_SONGS_MAX &&
                Etable_get(table->songs, index) != NULL)
        {
            table->effective_size = index + 1;
            ++index;
        }
    }

    return true;
}


Song* Song_table_get(Song_table* table, int index)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);

    if (index >= table->effective_size)
        return NULL;

    return Etable_get(table->songs, index);
}


void Song_table_set_existent(Song_table* table, int index, bool existent)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);

    Bit_array_set(table->existents, index, existent);

    return;
}


bool Song_table_get_existent(Song_table* table, int index)
{
    assert(table != NULL);
    assert(index < KQT_SONGS_MAX);

    return Bit_array_get(table->existents, index);
}


void del_Song_table(Song_table* table)
{
    if (table == NULL)
        return;

    del_Etable(table->songs);
    del_Bit_array(table->existents);
    memory_free(table);

    return;
}


