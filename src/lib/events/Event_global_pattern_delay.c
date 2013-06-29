

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
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_master_decl.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_pattern_delay_process(Master_params* master_params, Playdata* global_state, Value* value)
{
    assert(master_params != NULL || global_state != NULL);
    assert(value != NULL);

    if (master_params != NULL)
    {
        return false;
    }

    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    global_state->delay_event_index = global_state->event_index;
    Tstamp_copy(&global_state->delay_left, &value->value.Tstamp_type);
    return true;
}


