

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_NOISE_STATE_H
#define KQT_NOISE_STATE_H


#include <decl.h>
#include <player/devices/Voice_state.h>

#include <stdint.h>
#include <stdlib.h>


Device_state* new_Noise_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);


Voice_state_get_size_func Noise_vstate_get_size;

void Noise_vstate_init(Voice_state* vstate, const Proc_state* proc_state);


#endif // KQT_NOISE_STATE_H


