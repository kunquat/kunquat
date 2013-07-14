

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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
#include <Event_control_decl.h>
#include <Event_type.h>
#include <File_base.h>
#include <General_state.h>
#include <player/Master_params.h>
#include <Value.h>
#include <xassert.h>


bool Event_control_infinite_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_BOOL);

    if (!gstate->global)
        return false;

    Master_params* master_params = (Master_params*)gstate;
    master_params->is_infinite = value->value.bool_type;

    return true;
}


