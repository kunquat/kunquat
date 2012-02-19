

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
#include <stdbool.h>

#include <Event_common.h>
#include <Event_control.h>
#include <Event_control_set_goto_section.h>
#include <General_state.h>
#include <Playdata.h>
#include <Value.h>
#include <xassert.h>


#if 0
static Event_field_desc set_goto_section_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = -1,
        .max.field.integral_type = KQT_SECTIONS_MAX - 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};
#endif


Event_create_constructor(Event_control,
                         EVENT_CONTROL_SET_GOTO_SECTION,
                         set_goto_section);


bool Event_control_set_goto_section_process(General_state* gstate,
                                            Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT || !gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->goto_set_section = value->value.int_type;
    return true;
}


