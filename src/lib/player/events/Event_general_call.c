

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_general_decl.h>

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/General_state.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_general_call_bool_process(General_state* gstate, const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    return value->type == VALUE_TYPE_BOOL;
}


bool Event_general_call_float_process(General_state* gstate, const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    return value->type == VALUE_TYPE_FLOAT;
}


bool Event_general_call_int_process(General_state* gstate, const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    return value->type == VALUE_TYPE_INT;
}


