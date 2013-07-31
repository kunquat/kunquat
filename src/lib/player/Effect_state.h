

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EFFECT_STATE_H
#define K_EFFECT_STATE_H


#include <stdbool.h>
#include <stdlib.h>

#include <player/Device_state.h>


typedef struct Effect_state
{
    Device_state parent;

    bool bypass;
} Effect_state;


/**
 * Resets the Effect state.
 *
 * \param eff_state   The Effect state -- must not be \c NULL.
 */
void Effect_state_reset(Effect_state* eff_state);


#endif // K_EFFECT_STATE_H


