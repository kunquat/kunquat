

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <math.h>

#include <Event_common.h>
#include <Event_general.h>
#include <Event_general_call_float.h>
#include <General_state.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc call_float_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = INFINITY
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_general,
                         EVENT_GENERAL_CALL_FLOAT,
                         call_float);


bool Event_general_call_float_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, call_float_desc, NULL, state);
    return !state->error;
}


