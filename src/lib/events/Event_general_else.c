

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
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
#include <Event_general_else.h>
#include <General_state.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_general,
                         EVENT_GENERAL_ELSE,
                         else);


bool Event_general_else_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    (void)value;
    ++gstate->cond_level_index;
    assert(gstate->cond_level_index >= 0);
    if (gstate->cond_level_index < COND_LEVELS_MAX)
    {
        gstate->cond_levels[gstate->cond_level_index].cond_for_exec = false;
        if (gstate->last_cond_match + 1 == gstate->cond_level_index &&
                gstate->cond_levels[gstate->cond_level_index].cond_for_exec ==
                gstate->cond_levels[gstate->cond_level_index].evaluated_cond)
        {
            ++gstate->last_cond_match;
        }
    }
    return true;
}


