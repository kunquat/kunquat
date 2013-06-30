

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
#include <Playdata.h>
#include <Value.h>
#include <xassert.h>


bool Event_control_infinite_process(General_state* mgstate, General_state* gstate, Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    assert(value != NULL);

    if (mgstate != NULL)
        return false;

    if (value->type != VALUE_TYPE_BOOL)
    {
        return false;
    }
    if (!gstate->global)
    {
        return true;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->infinite = value->value.bool_type;
    return true;
}


