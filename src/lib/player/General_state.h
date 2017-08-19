

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_GENERAL_STATE_H
#define KQT_GENERAL_STATE_H


#include <decl.h>
#include <player/Active_names.h>
#include <player/Env_state.h>

#include <stdbool.h>


#define COND_LEVELS_MAX 32


typedef struct Cond_level
{
    bool cond_for_exec;
    bool evaluated_cond;
} Cond_level;


struct General_state
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
    Env_state* estate;
    Active_names* active_names;

    const Module* module;
};


/**
 * Pre-initialise the General state.
 *
 * This function must be called before General_state_init on a given state.
 *
 * \param state   The General state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
General_state* General_state_preinit(General_state* state);


/**
 * Initialise the General state.
 *
 * \param state    The General state -- must not be \c NULL.
 * \param global   \c true if and only if \a state is global.
 * \param estate   The Environment state -- must not be \c NULL.
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The parameter \a state if successful, or \c NULL if
 *           memory allocation failed. The function
 *           General_state_deinit should be called even if this
 *           function fails.
 */
General_state* General_state_init(
        General_state* state, bool global, Env_state* estate, const Module* module);


/**
 * Tell whether events connected to the General state should be processed.
 *
 * \param state   The General state -- must not be \c NULL.
 *
 * \return   \c true if events should be processed, otherwise \c false.
 */
bool General_state_events_enabled(General_state* state);


/**
 * Reset the General state.
 *
 * \param state    The General state -- must not be \c NULL.
 */
void General_state_reset(General_state* state);


/**
 * Deinitialise the General state.
 *
 * \param state    The General state -- must not be \c NULL.
 */
void General_state_deinit(General_state* state);


#endif // KQT_GENERAL_STATE_H


