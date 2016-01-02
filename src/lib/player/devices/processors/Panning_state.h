

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PANNING_STATE_H
#define K_PANNING_STATE_H


#include <decl.h>
#include <player/devices/Device_state.h>
#include <string/key_pattern.h>

#include <stdint.h>


Device_state* new_Panning_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

bool Panning_pstate_set_panning(
        Device_state* dstate, const Key_indices indices, double value);


#endif // K_PANNING_STATE_H


