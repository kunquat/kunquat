

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


#ifndef K_GENERAL_STATE_H
#define K_GENERAL_STATE_H


#include <stdbool.h>


typedef struct General_state
{
    bool pause;
    int pattern;
} General_state;


/**
 * Initialises the General state.
 *
 * \param state   The General state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
General_state* General_state_init(General_state* state);


#endif // K_GENERAL_STATE_H


