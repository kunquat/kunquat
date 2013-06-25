

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
#include <File_base.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_shift_scale_intervals_process(
        Playdata* global_state,
        Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);
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


