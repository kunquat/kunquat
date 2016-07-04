

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_general_decl.h>

#include <debug/assert.h>
#include <init/Env_var.h>
#include <player/events/Event_common.h>
#include <player/General_state.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool Event_general_cond_process(General_state* gstate, const Value* value)
{
    rassert(gstate != NULL);
    rassert(value != NULL);

    if (value->type != VALUE_TYPE_BOOL)
        return false;

    if (gstate->cond_level_index >= COND_LEVELS_MAX - 1)
        return true;

    gstate->cond_levels[gstate->cond_level_index + 1].evaluated_cond =
            value->value.bool_type;
    return true;
}


bool Event_general_if_process(General_state* gstate, const Value* value)
{
    rassert(gstate != NULL);
    ignore(value);

    ++gstate->cond_level_index;
    rassert(gstate->cond_level_index >= 0);
    if (gstate->cond_level_index < COND_LEVELS_MAX)
    {
        gstate->cond_levels[gstate->cond_level_index].cond_for_exec = true;
        if (gstate->last_cond_match + 1 == gstate->cond_level_index &&
                gstate->cond_levels[gstate->cond_level_index].cond_for_exec ==
                gstate->cond_levels[gstate->cond_level_index].evaluated_cond)
            ++gstate->last_cond_match;
    }

    //fprintf(stderr, "if: %d %d\n", gstate->cond_level_index,
    //                               gstate->last_cond_match);
    return true;
}


bool Event_general_else_process(General_state* gstate, const Value* value)
{
    rassert(gstate != NULL);
    ignore(value);

    ++gstate->cond_level_index;
    rassert(gstate->cond_level_index >= 0);
    if (gstate->cond_level_index < COND_LEVELS_MAX)
    {
        gstate->cond_levels[gstate->cond_level_index].cond_for_exec = false;
        if (gstate->last_cond_match + 1 == gstate->cond_level_index &&
                gstate->cond_levels[gstate->cond_level_index].cond_for_exec ==
                gstate->cond_levels[gstate->cond_level_index].evaluated_cond)
            ++gstate->last_cond_match;
    }

    return true;
}


bool Event_general_end_if_process(General_state* gstate, const Value* value)
{
    rassert(gstate != NULL);
    ignore(value);

    if (gstate->cond_level_index >= 0)
        --gstate->cond_level_index;

    if (gstate->cond_level_index < gstate->last_cond_match)
    {
        --gstate->last_cond_match;
        rassert(gstate->cond_level_index == gstate->last_cond_match);
    }

#if 0
    gstate->cond_exec_enabled = false;
#endif
    return true;
}


