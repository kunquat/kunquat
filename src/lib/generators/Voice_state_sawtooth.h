

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_VOICE_STATE_SAWTOOTH_H
#define K_VOICE_STATE_SAWTOOTH_H


#include <Voice_state.h>


typedef struct Voice_state_sawtooth
{
    Voice_state parent;
    double phase;
} Voice_state_sawtooth;


/**
 * Initialises the Sawtooth Instrument parameters.
 *
 * \param sawtooth   The Sawtooth parameters -- must not be \c NULL.
 */
void Voice_state_sawtooth_init(Voice_state* state);


#endif // K_VOICE_STATE_SAWTOOTH_H


