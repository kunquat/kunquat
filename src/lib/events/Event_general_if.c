

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

#include <Event_common.h>
#include <Event_general.h>
#include <Event_general_if.h>
#include <General_state.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc if_desc[] =
{
    {
        .type = EVENT_FIELD_BOOL
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_general,
                         EVENT_GENERAL_IF,
                         if);


bool Event_general_if_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, if_desc, data, state);
    if (state->error)
    {
        return false;
    }
    gstate->cond_exec_enabled = true;
    gstate->cond_for_exec = data[0].field.bool_type;
    return true;
}


