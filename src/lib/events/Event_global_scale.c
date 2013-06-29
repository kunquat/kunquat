

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

#include <Event_common.h>
#include <Event_master_decl.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_set_scale_process(Master_params* master_params, Playdata* global_state, Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
        return false;

    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    global_state->scale = value->value.int_type;

    return true;
}


bool Event_global_set_scale_fixed_point_process(
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
    global_state->scale_fixed_point = value->value.int_type;

    return true;
}


bool Event_global_set_scale_offset_process(
        Master_params* master_params,
        Playdata* global_state,
        Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
        return false;

    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    if (global_state->scales == NULL)
    {
        return true;
    }
    Scale* scale = global_state->scales[global_state->scale];
    if (scale == NULL)
    {
        return true;
    }
    Scale_set_pitch_offset(scale, value->value.float_type);
    return true;
}


bool Event_global_mimic_scale_process(Master_params* master_params, Playdata* global_state, Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
        return false;

    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    if (global_state->scales == NULL)
    {
        return true;
    }
    Scale* scale = global_state->scales[global_state->scale];
    Scale* modifier = global_state->scales[value->value.int_type];
    if (scale == NULL || modifier == NULL)
    {
        return true;
    }
    Scale_retune_with_source(scale, modifier);
    return true;
}


bool Event_global_shift_scale_intervals_process(
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
    if (global_state->scales == NULL)
    {
        return true;
    }
    Scale* scale = global_state->scales[global_state->scale];
    if (scale == NULL ||
            Scale_get_note_count(scale) <= value->value.int_type ||
            Scale_get_note_count(scale) <= global_state->scale_fixed_point)
    {
        return true;
    }
    Scale_retune(
            scale,
            value->value.int_type,
            global_state->scale_fixed_point);
    return true;
}


