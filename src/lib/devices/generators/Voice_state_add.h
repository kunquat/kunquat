

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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


#include <player/Voice_state.h>


#define HARMONICS_MAX 32


typedef struct Add_tone_state
{
    double phase;
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
    double mod_env_pos;
    int mod_env_next_node;
    double mod_env_value;
    double mod_env_update;
    double mod_env_scale;

    Add_tone_state tones[HARMONICS_MAX];
    Add_tone_state mod_tones[HARMONICS_MAX];
} Voice_state_add;


#endif // K_VOICE_STATE_ADD_H


