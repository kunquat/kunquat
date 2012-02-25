

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


#ifndef K_GENERAL_STATE_H
#define K_GENERAL_STATE_H


#include <stdbool.h>

#include <Active_names.h>
#include <Environment.h>


#define COND_LEVELS_MAX 32


typedef struct Cond_level
{
    bool cond_for_exec;
    bool evaluated_cond;
} Cond_level;


typedef struct General_state
{
    bool global;
    bool pause;
    int cond_level_index;
    int last_cond_match;
    Cond_level cond_levels[COND_LEVELS_MAX];
#if 0
    bool cond_exec_enabled;
    bool cond_for_exec;
    bool evaluated_cond;
#endif
    Environment* env;
    Active_names* active_names;
} General_state;


/**
 * Initialises the General state.
 *
 * \param state    The General state -- must not be \c NULL.
 * \param global   \c true if and only if \a state is global.
 * \param env      The Environment -- must not be \c NULL.
 *
 * \return   The parameter \a state if successful, or \c NULL if
 *           memory allocation failed. The function
 *           General_state_uninit should be called even if this
 *           function fails.
 */
General_state* General_state_init(General_state* state,
                                  bool global,
                                  Environment* env);


/**
 * Tells whether events connected to the General state should be processed.
 *
 * \param state   The General state -- must not be \c NULL.
 *
 * \return   \c true if events should be processed, otherwise \c false.
 */
bool General_state_events_enabled(General_state* state);


/**
 * Resets the General state.
 *
 * \param state    The General state -- must not be \c NULL.
 */
void General_state_reset(General_state* state);


/**
 * Uninitialises the General state.
 *
 * \param state    The General state -- must not be \c NULL.
 */
void General_state_uninit(General_state* state);


#endif // K_GENERAL_STATE_H


