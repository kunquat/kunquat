

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


typedef struct General_state
{
    bool global;
    bool pause;
    bool cond_exec_enabled;
    bool cond_for_exec;
    bool evaluated_cond;
} General_state;


/**
 * Initialises the General state.
 *
 * \param state   The General state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
General_state* General_state_init(General_state* state, bool global);


/**
 * Tells whether events connected to the General state should be processed.
 *
 * \param state   The General state -- must not be \c NULL.
 *
 * \return   \c true if events should be processed, otherwise \c false.
 */
bool General_state_events_enabled(General_state* state);


#endif // K_GENERAL_STATE_H


