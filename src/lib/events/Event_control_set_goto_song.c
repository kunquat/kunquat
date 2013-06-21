

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
#include <Event_control_set_goto_song.h>
#include <General_state.h>
#include <Playdata.h>
#include <Value.h>
#include <xassert.h>


bool Event_control_set_goto_song_process(
        General_state* gstate,
        Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT || !gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->goto_set_subsong = value->value.int_type;
    return true;
}


