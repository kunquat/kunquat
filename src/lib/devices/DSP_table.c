

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
#include <stdbool.h>

#include <Bit_array.h>
#include <containers/Etable.h>
#include <devices/DSP.h>
#include <devices/DSP_table.h>
#include <memory.h>
#include <xassert.h>


struct DSP_table
{
    int size;
    Etable* dsps;
    Bit_array* existents;
};


DSP_table* new_DSP_table(int size)
{
    assert(size > 0);

    DSP_table* table = memory_alloc_item(DSP_table);
    if (table == NULL)
        return NULL;

    table->dsps = NULL;
    table->existents = NULL;

    table->dsps = new_Etable(size, (void (*)(void*))del_DSP);
    table->existents = new_Bit_array(size);
    if (table->dsps == NULL || table->existents == NULL)
    {
        del_DSP_table(table);
        return NULL;
    }

    table->size = size;

    return table;
}


void DSP_table_set_existent(DSP_table* table, int index, bool existent)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    Bit_array_set(table->existents, index, existent);

    DSP* dsp = Etable_get(table->dsps, index);
    if (dsp != NULL)
        Device_set_existent((Device*)dsp, existent);

    return;
}


bool DSP_table_set_dsp(DSP_table* table, int index, DSP* dsp)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    assert(dsp != NULL);

    if (!Etable_set(table->dsps, index, dsp))
        return false;

    Device_set_existent((Device*)dsp, Bit_array_get(table->existents, index));

    return true;
}


DSP* DSP_table_get_dsp(const DSP_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    return Etable_get(table->dsps, index);
}


void DSP_table_remove_dsp(DSP_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);

    Etable_remove(table->dsps, index);

    return;
}


void DSP_table_clear(DSP_table* table)
{
    assert(table != NULL);

    Etable_clear(table->dsps);

    return;
}


void del_DSP_table(DSP_table* table)
{
    if (table == NULL)
        return;

    del_Etable(table->dsps);
    del_Bit_array(table->existents);
    memory_free(table);

    return;
}


