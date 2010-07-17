

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <Etable.h>
#include <DSP.h>
#include <DSP_table.h>
#include <xassert.h>
#include <xmemory.h>


struct DSP_table
{
    int size;
    Etable* confs;
    Etable* dsps;
};


DSP_table* new_DSP_table(int size)
{
    assert(size > 0);
    DSP_table* table = xalloc(DSP_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->confs = NULL;
    table->dsps = NULL;
    table->confs = new_Etable(size, (void (*)(void*))del_DSP_conf);
    if (table->confs == NULL)
    {
        del_DSP_table(table);
        return NULL;
    }
    table->dsps = new_Etable(size, (void (*)(void*))del_DSP);
    if (table->dsps == NULL)
    {
        del_DSP_table(table);
        return NULL;
    }
    table->size = size;
    return table;
}


bool DSP_table_set_conf(DSP_table* table, int index, DSP_conf* conf)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    assert(conf != NULL);
    if (!Etable_set(table->confs, index, conf))
    {
        return false;
    }
    DSP* dsp = Etable_get(table->dsps, index);
    if (dsp != NULL)
    {
        DSP_set_conf(dsp, conf);
    }
    return true;
}


DSP_conf* DSP_table_get_conf(DSP_table* table, int index)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    DSP_conf* conf = Etable_get(table->confs, index);
    if (conf == NULL)
    {
        conf = new_DSP_conf();
        if (conf == NULL)
        {
            return NULL;
        }
        if (!DSP_table_set_conf(table, index, conf))
        {
            del_DSP_conf(conf);
            return NULL;
        }
    }
    assert(conf != NULL);
    return conf;
}


bool DSP_table_set_dsp(DSP_table* table, int index, DSP* dsp)
{
    assert(table != NULL);
    assert(index >= 0);
    assert(index < table->size);
    assert(dsp != NULL);
    DSP_conf* conf = Etable_get(table->confs, index);
    if (conf == NULL)
    {
        conf = new_DSP_conf();
        if (conf == NULL)
        {
            return false;
        }
        if (!Etable_set(table->confs, index, conf))
        {
            del_DSP_conf(conf);
            return false;
        }
    }
    if (!Etable_set(table->dsps, index, dsp))
    {
        return false;
    }
    DSP_set_conf(dsp, conf);
    return true;
}


DSP* DSP_table_get_dsp(DSP_table* table, int index)
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
    Etable_clear(table->confs);
    return;
}


void del_DSP_table(DSP_table* table)
{
    assert(table != NULL);
    if (table->confs != NULL)
    {
        del_Etable(table->confs);
    }
    if (table->dsps != NULL)
    {
        del_Etable(table->dsps);
    }
    xfree(table);
    return;
}


