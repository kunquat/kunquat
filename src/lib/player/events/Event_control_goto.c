

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
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

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/events/Event_control_decl.h>
#include <player/General_state.h>
#include <Value.h>


bool Event_control_goto_process(General_state* gstate, const Value* value)
{
    assert(gstate != NULL);
    (void)value;

    return false;

#if 0
    if (!gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->goto_trigger = true;
    global_state->goto_subsong = global_state->goto_set_subsong;
    global_state->goto_section = global_state->goto_set_section;
    Tstamp_copy(&global_state->goto_row, &global_state->goto_set_row);
    return true;
#endif
}


bool Event_control_set_goto_row_process(General_state* gstate, const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);

    return false;

#if 0
    if (!gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    Tstamp_copy(&global_state->goto_set_row, &value->value.Tstamp_type);
    return true;
#endif
}


bool Event_control_set_goto_section_process(
        General_state* gstate,
        const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);

    return false;

#if 0
    if (value->type != VALUE_TYPE_INT || !gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->goto_set_section = value->value.int_type;
    return true;
#endif
}


bool Event_control_set_goto_song_process(
        General_state* gstate,
        const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);

    return false;

#if 0
    if (value->type != VALUE_TYPE_INT || !gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->goto_set_subsong = value->value.int_type;
    return true;
#endif
}


