

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
#include <Event_general_end_if.h>
#include <General_state.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc end_if_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_general,
                         EVENT_GENERAL_END_IF,
                         end_if);


bool Event_general_end_if_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    (void)fields;
    gstate->cond_exec_enabled = false;
    return true;
}


