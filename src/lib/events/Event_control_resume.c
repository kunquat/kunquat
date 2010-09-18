

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <General_state.h>
#include <xassert.h>


bool Event_control_resume_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    (void)fields;
    gstate->pause = false;
    return true;
}


