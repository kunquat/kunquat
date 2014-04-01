

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Env_var.h>
#include <General_state.h>
#include <player/events/Event_common.h>
#include <player/events/Event_general_decl.h>
#include <Value.h>
#include <xassert.h>


bool Event_general_cond_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_BOOL)
    {
        return false;
    }
    if (gstate->cond_level_index >= COND_LEVELS_MAX - 1)
    {
        return true;
    }
    gstate->cond_levels[gstate->cond_level_index + 1].evaluated_cond =
            value->value.bool_type;
    return true;
}


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


bool Event_general_end_if_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    (void)value;
    if (gstate->cond_level_index >= 0)
    {
        --gstate->cond_level_index;
    }
    if (gstate->cond_level_index < gstate->last_cond_match)
    {
        --gstate->last_cond_match;
        assert(gstate->cond_level_index == gstate->last_cond_match);
    }
#if 0
    gstate->cond_exec_enabled = false;
#endif
    return true;
}


