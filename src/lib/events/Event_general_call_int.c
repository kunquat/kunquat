

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_general.h>
#include <Event_general_call_int.h>
#include <General_state.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


#if 0
static Event_field_desc call_int_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = INT64_MIN,
        .max.field.integral_type = INT64_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};
#endif


Event_create_constructor(Event_general,
                         EVENT_GENERAL_CALL_INT,
                         call_int);


bool Event_general_call_int_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    return value->type == VALUE_TYPE_INT;
}


