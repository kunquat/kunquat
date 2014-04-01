

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

#include <Bit_array.h>
#include <containers/Etable.h>
#include <Gen_table.h>
#include <Generator.h>
#include <memory.h>
#include <xassert.h>


struct Gen_table
{
    int size;
    Etable* gens;
    Bit_array* existents;
};


Gen_table* new_Gen_table(int size)
{
    assert(size > 0);

    Gen_table* table = memory_alloc_item(Gen_table);
    if (table == NULL)
        return NULL;

    table->gens = NULL;
    table->existents = NULL;

    table->gens = new_Etable(size, (void (*)(void*))del_Generator);
    table->existents = new_Bit_array(size);
    if (table->gens == NULL || table->existents == NULL)
    {
        del_Gen_table(table);
        return NULL;
    }

    table->size = size;

    return table;
}


void Gen_table_set_existent(Gen_table* table, int index, bool existent)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    Bit_array_set(table->existents, index, existent);

    Generator* gen = Etable_get(table->gens, index);
    if (gen != NULL)
        Device_set_existent((Device*)gen, existent);

    return;
}


bool Gen_table_set_gen(Gen_table* table, int index, Generator* gen)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    assert(gen != NULL);

    if (!Etable_set(table->gens, index, gen))
        return false;

    Device_set_existent((Device*)gen, Bit_array_get(table->existents, index));

    return true;
}


Generator* Gen_table_get_gen(Gen_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    return Etable_get(table->gens, index);
}


void Gen_table_remove_gen(Gen_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    Etable_remove(table->gens, index);

    return;
}


void Gen_table_clear(Gen_table* table)
{
    assert(table != NULL);

    Etable_clear(table->gens);

    return;
}


void del_Gen_table(Gen_table* table)
{
    if (table == NULL)
        return;

    del_Etable(table->gens);
    del_Bit_array(table->existents);
    memory_free(table);

    return;
}


