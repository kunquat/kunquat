

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Jump_context.h>

#include <debug/assert.h>
#include <memory.h>

#include <stdlib.h>


Jump_context* new_Jump_context(void)
{
    Jump_context* jc = memory_alloc_item(Jump_context);
    if (jc == NULL)
        return NULL;

    jc->piref.pat = -1;
    jc->piref.inst = -1;
    Tstamp_set(&jc->row, 0, 0);
    jc->ch_num = 0;
    jc->order = 0;

    jc->counter = 0;

    jc->target_piref.pat = -1;
    jc->target_piref.inst = -1;
    Tstamp_set(&jc->target_row, 0, 0);

    return jc;
}


int Jump_context_cmp(const Jump_context* jc1, const Jump_context* jc2)
{
    rassert(jc1 != NULL);
    rassert(jc2 != NULL);

    int diff = Pat_inst_ref_cmp(&jc1->piref, &jc2->piref);
    if (diff != 0)
        return diff;

    diff = Tstamp_cmp(&jc1->row, &jc2->row);
    if (diff != 0)
        return diff;

    if (jc1->ch_num < jc2->ch_num)
        return -1;
    else if (jc1->ch_num > jc2->ch_num)
        return 1;
    else if (jc1->order < jc2->order)
        return -1;
    else if (jc1->order > jc2->order)
        return 1;

    return 0;
}


void del_Jump_context(Jump_context* jc)
{
    if (jc == NULL)
        return;

    memory_free(jc);
    return;
}


