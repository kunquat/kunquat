

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

#include <Event_common.h>
#include <Event_general.h>
#include <Event_general_if.h>
#include <General_state.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


#if 0
static Event_field_desc if_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};
#endif


Event_create_constructor(Event_general,
                         EVENT_GENERAL_IF,
                         if);


bool Event_general_if_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    (void)value;
    ++gstate->cond_level_index;
    assert(gstate->cond_level_index >= 0);
    if (gstate->cond_level_index < COND_LEVELS_MAX)
    {
        gstate->cond_levels[gstate->cond_level_index].cond_for_exec = true;
        if (gstate->last_cond_match + 1 == gstate->cond_level_index &&
                gstate->cond_levels[gstate->cond_level_index].cond_for_exec ==
                gstate->cond_levels[gstate->cond_level_index].evaluated_cond)
        {
            ++gstate->last_cond_match;
        }
    }
    //fprintf(stderr, "if: %d %d\n", gstate->cond_level_index,
    //                               gstate->last_cond_match);
    return true;
}


