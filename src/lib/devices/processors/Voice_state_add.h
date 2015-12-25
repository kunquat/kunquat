

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
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


#include <player/devices/Voice_state.h>


#define HARMONICS_MAX 32


typedef struct Add_tone_state
{
    double phase[2];
} Add_tone_state;


typedef struct Voice_state_add
{
    Voice_state parent;
    int tone_limit;
    Add_tone_state tones[HARMONICS_MAX];
} Voice_state_add;


#endif // K_VOICE_STATE_ADD_H


