

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


#ifndef KQT_VOLUME_STATE_H
#define KQT_VOLUME_STATE_H


#include <decl.h>
#include <player/devices/Device_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>


Device_state* new_Volume_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

bool Volume_pstate_set_volume(
        Device_state* dstate, const Key_indices indices, double value);


Voice_state_get_size_func Volume_vstate_get_size;

void Volume_vstate_init(Voice_state* vstate, const Proc_state* proc_state);


#endif // KQT_VOLUME_STATE_H


