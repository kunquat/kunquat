

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
#include <stdbool.h>

#include <Event.h>
#include <Event_common.h>
#include <Event_control.h>
#include <Event_control_receive_event.h>
#include <Event_names.h>
#include <Event_type.h>
#include <File_base.h>
#include <General_state.h>
#include <Playdata.h>
#include <xassert.h>


static Event_field_desc receive_event_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_control,
                         EVENT_CONTROL_RECEIVE_EVENT,
                         receive_event);


bool Event_control_receive_event(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    if (fields == NULL || !gstate->global)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, receive_event_desc, data, state);
    if (state->error)
    {
        return false;
    }
    char event_name[EVENT_NAME_MAX + 1] = "";
    state = READ_STATE_AUTO;
    read_string(data[0].field.string_type, event_name,
                EVENT_NAME_MAX + 1, state);
    if (state->error)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    if (global_state->event_filter != NULL)
    {
        Event_names_set_pass(global_state->event_filter, event_name, true);
    }
    return true;
}


