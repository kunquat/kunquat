

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <Pat_table.h>
#include <Pattern.h>
#include <kunquat/limits.h>

#include <xmemory.h>


Pat_table* new_Pat_table(int size)
{
    assert(size > 0);
    Pat_table* table = xalloc(Pat_table);
    if (table == NULL)
    {
        return NULL;
    }
    table->pats = new_Etable(size, (void (*)(void*))del_Pattern);
    if (table->pats == NULL)
    {
        xfree(table);
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
    assert(table != NULL);
    del_Etable(table->pats);
    xfree(table);
    return;
}


