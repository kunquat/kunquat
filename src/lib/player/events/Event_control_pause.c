

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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
#include <stdio.h>

#include <player/events/Event_common.h>
#include <player/events/Event_control_decl.h>
#include <player/General_state.h>
#include <Value.h>
#include <xassert.h>


bool Event_control_pause_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    (void)value;

    if (!gstate->global)
        return false;

    gstate->pause = true;
    return true;
}


bool Event_control_resume_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    (void)value;

    if (!gstate->global)
        return false;

    gstate->pause = false;
    return true;
}


