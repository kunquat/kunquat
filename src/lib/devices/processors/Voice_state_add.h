

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


#include <player/Time_env_state.h>
#include <player/Voice_state.h>


#define HARMONICS_MAX 32


typedef struct Add_tone_state
{
    double phase[2];
#if 0
    uint64_t pos;
    double pos_rem;
    uint64_t rel_pos;
    double rel_pos_rem;
    Filter_state lowpass_state[2];
#endif
} Add_tone_state;


typedef struct Voice_state_add
{
    Voice_state parent;
//    double phase;
    int tone_limit;
    int mod_tone_limit;

    bool mod_active;

    Time_env_state mod_env_state;

    Add_tone_state tones[HARMONICS_MAX];
    Add_tone_state mod_tones[HARMONICS_MAX];
} Voice_state_add;


#endif // K_VOICE_STATE_ADD_H


