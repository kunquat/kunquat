

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VOICE_STATE_ADD_H
#define K_VOICE_STATE_ADD_H


#include <Voice_state.h>


#define HARMONICS_MAX 32


typedef struct Add_tone_state
{
    uint64_t pos;
    double pos_rem;
    uint64_t rel_pos;
    double rel_pos_rem;
    Filter_state lowpass_state[2];
} Add_tone_state;


typedef struct Voice_state_add
{
    Voice_state parent;
    Add_tone_state tones[HARMONICS_MAX];
} Voice_state_add;


#endif // K_VOICE_STATE_ADD_H


