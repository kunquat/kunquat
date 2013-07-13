

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
#include <stdbool.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_master_decl.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_set_jump_counter_process(
        Master_params* master_params,
        Playdata* global_state,
        Value* value)
{
    assert(master_params != NULL || global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
    {
        assert(value->type == VALUE_TYPE_INT);

        master_params->jump_counter = value->value.int_type;

        return true;
    }

    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    global_state->jump_set_counter = value->value.int_type;
    return true;
}


bool Event_global_set_jump_row_process(Master_params* master_params, Playdata* global_state, Value* value)
{
    assert(master_params != NULL || global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
    {
        assert(value->type == VALUE_TYPE_TSTAMP);
        Tstamp_copy(&master_params->jump_target_row, &value->value.Tstamp_type);
        return true;
    }

    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    Tstamp_copy(&global_state->jump_set_row, &value->value.Tstamp_type);
    return true;
}


bool Event_global_set_jump_pat_inst_process(
        Master_params* master_params,
        Playdata* global_state,
        Value* value)
{
    assert(master_params != NULL);
    (void)global_state;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_PAT_INST_REF);

    master_params->jump_target_piref = value->value.Pat_inst_ref_type;

    return true;
}


#if 0
bool Event_global_set_jump_section_process(
        Master_params* master_params,
        Playdata* global_state,
        Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
        return false;

    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    global_state->jump_set_section = value->value.int_type;
    return true;
}


bool Event_global_set_jump_song_process(
        Master_params* master_params,
        Playdata* global_state,
        Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
        return false;

    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    global_state->jump_set_subsong = value->value.int_type;
    return true;
}
#endif


