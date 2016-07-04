

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


#include <init/devices/Proc_table.h>

#include <containers/Bit_array.h>
#include <containers/Etable.h>
#include <debug/assert.h>
#include <init/devices/Processor.h>
#include <memory.h>

#include <stdbool.h>
#include <stdlib.h>


struct Proc_table
{
    int size;
    Etable* procs;
    Bit_array* existents;
};


Proc_table* new_Proc_table(int size)
{
    rassert(size > 0);

    Proc_table* table = memory_alloc_item(Proc_table);
    if (table == NULL)
        return NULL;

    table->procs = NULL;
    table->existents = NULL;

    table->procs = new_Etable(size, (void (*)(void*))del_Processor);
    table->existents = new_Bit_array(size);
    if (table->procs == NULL || table->existents == NULL)
    {
        del_Proc_table(table);
        return NULL;
    }

    table->size = size;

    return table;
}


void Proc_table_set_existent(Proc_table* table, int index, bool existent)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    Bit_array_set(table->existents, index, existent);

    Processor* proc = Etable_get(table->procs, index);
    if (proc != NULL)
        Device_set_existent((Device*)proc, existent);

    return;
}


bool Proc_table_set_proc(Proc_table* table, int index, Processor* proc)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);
    rassert(proc != NULL);

    if (!Etable_set(table->procs, index, proc))
        return false;

    Device_set_existent((Device*)proc, Bit_array_get(table->existents, index));

    return true;
}


const Processor* Proc_table_get_proc(const Proc_table* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    return Etable_get(table->procs, index);
}


Processor* Proc_table_get_proc_mut(Proc_table* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    return Etable_get(table->procs, index);
}


void Proc_table_remove_proc(Proc_table* table, int index)
{
    rassert(table != NULL);
    rassert(index >= 0);
    rassert(index < table->size);

    Etable_remove(table->procs, index);

    return;
}


void Proc_table_clear(Proc_table* table)
{
    rassert(table != NULL);

    Etable_clear(table->procs);

    return;
}


void del_Proc_table(Proc_table* table)
{
    if (table == NULL)
        return;

    del_Etable(table->procs);
    del_Bit_array(table->existents);
    memory_free(table);

    return;
}


