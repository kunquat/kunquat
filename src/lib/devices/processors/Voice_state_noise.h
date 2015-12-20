

/*
 * Authors: Tomi Jylhä-Ollila, 2010-2015
 *          Ossi Saresoja, Finland 2009
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VOICE_STATE_NOISE_H
#define K_VOICE_STATE_NOISE_H


#include <Filter.h>
#include <player/Voice_state.h>


#define NOISE_MAX 8


typedef struct Voice_state_noise
{
    Voice_state parent;
    double buf[2][NOISE_MAX];
} Voice_state_noise;


/**
 * Initialise the Noise processor parameters.
 *
 * \param noise   The Noise parameters -- must not be \c NULL.
 */
void Voice_state_noise_init(Voice_state* state);


#endif // K_VOICE_STATE_NOISE_H


