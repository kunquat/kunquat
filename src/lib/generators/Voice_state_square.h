

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


#ifndef K_VOICE_STATE_SQUARE_H
#define K_VOICE_STATE_SQUARE_H


#include <Voice_state.h>


typedef struct Voice_state_square
{
    Voice_state parent;
    double phase;
    double pulse_width;
} Voice_state_square;


/**
 * Initialises the Square Instrument parameters.
 *
 * \param square   The Square parameters -- must not be \c NULL.
 */
void Voice_state_square_init(Voice_state* state);


#endif // K_VOICE_STATE_SQUARE_H


