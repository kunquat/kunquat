

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_AU_STATE_H
#define K_AU_STATE_H


#include <player/Device_state.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Au_state
{
    Device_state parent;

    bool bypass;
    double sustain; // 0 = no sustain, 1.0 = full sustain
} Au_state;


/**
 * Reset the Audio unit state.
 *
 * \param au_state   The Audio unit state -- must not be \c NULL.
 */
void Au_state_reset(Au_state* au_state);


#endif // K_AU_STATE_H


