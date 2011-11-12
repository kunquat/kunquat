

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

#include <Event_control_goto.h>
#include <General_state.h>
#include <Playdata.h>
#include <xassert.h>


#if 0
static Event_field_desc goto_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};
#endif


bool Event_control_goto_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    (void)fields;
    if (!gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->goto_trigger = true;
    global_state->goto_subsong = global_state->goto_set_subsong;
    global_state->goto_section = global_state->goto_set_section;
    Reltime_copy(&global_state->goto_row, &global_state->goto_set_row);
    return true;
}


