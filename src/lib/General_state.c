

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>

#include <General_state.h>
#include <xassert.h>


General_state* General_state_init(General_state* state,
                                  bool global,
                                  Environment* env)
{
    assert(state != NULL);
    assert(env != NULL);
    state->global = global;
    state->env = env;
    state->active_names = new_Active_names();
    if (state->active_names == NULL)
    {
        return NULL;
    }
    General_state_reset(state);
    return state;
}


bool General_state_events_enabled(General_state* state)
{
    assert(state != NULL);
#if 0
    if (state->global)
        fprintf(stderr, "enabled: %d, exec: %d, evaluated: %d\n",
                        (int)state->cond_exec_enabled,
                        (int)state->cond_for_exec,
                        (int)state->evaluated_cond);
#endif
    return state->cond_level_index == state->last_cond_match;
#if 0
    return state->cond_levels[state->cond_level_index].cond_for_exec ==
            state->cond_levels[state->cond_level_index].evaluated_cond;
#endif
}


void General_state_reset(General_state* state)
{
    assert(state != NULL);
    state->pause = false;
    state->cond_level_index = -1;
    state->last_cond_match = -1;
    for (int i = 0; i < COND_LEVELS_MAX; ++i)
    {
        state->cond_levels[i].cond_for_exec = false;
        state->cond_levels[i].evaluated_cond = true;
    }
#if 0
    state->cond_exec_enabled = false;
    state->cond_for_exec = false;
    state->evaluated_cond = false;
#endif
    if (state->global)
    {
        // All states contain the same environment
        // and all states should be reset together,
        // so let's reset the environment only once.
        Environment_reset(state->env);
    }
    Active_names_reset(state->active_names);
    return;
}


void General_state_uninit(General_state* state)
{
    assert(state != NULL);
    del_Active_names(state->active_names);
    state->active_names = NULL;
    return;
}


