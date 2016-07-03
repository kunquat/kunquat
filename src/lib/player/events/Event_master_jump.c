

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


#include <player/events/Event_master_decl.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <player/events/Event_common.h>
#include <Value.h>

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


bool Event_master_set_jump_counter_process(
        Master_params* master_params, const Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    master_params->jump_counter = (int16_t)value->value.int_type;

    return true;
}


bool Event_master_set_jump_row_process(
        Master_params* master_params, const Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&master_params->jump_target_row, &value->value.Tstamp_type);

    return true;
}


bool Event_master_set_jump_pat_inst_process(
        Master_params* master_params, const Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_PAT_INST_REF);

    master_params->jump_target_piref = value->value.Pat_inst_ref_type;

    return true;
}


bool Event_master_jump_process(Master_params* master_params, const Value* value)
{
    assert(master_params != NULL);
    ignore(value);

    if (master_params->jump_counter <= 0)
        return true;

    // Get a new Jump context
    AAnode* handle = Jump_cache_acquire_context(master_params->jump_cache);
    if (handle == NULL)
    {
        fprintf(stderr, "Error: Out of jump contexts!\n");
        return false;
    }
    Jump_context* jc = AAnode_get_data(handle);

    // Init context
    jc->piref = master_params->cur_pos.piref;
    Tstamp_copy(&jc->row, &master_params->cur_pos.pat_pos);
    jc->ch_num = master_params->cur_ch;
    jc->order = master_params->cur_trigger;

    jc->counter = master_params->jump_counter;

    jc->target_piref = master_params->jump_target_piref;
    Tstamp_copy(&jc->target_row, &master_params->jump_target_row);

    // Store new context handle
    Active_jumps_add_context(master_params->active_jumps, handle);

    master_params->do_jump = true;

    return true;
}


