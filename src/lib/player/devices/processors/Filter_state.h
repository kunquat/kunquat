

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FILTER_STATE_H
#define KQT_FILTER_STATE_H


#include <init/devices/Device_impl.h>
#include <player/devices/Device_state.h>
#include <player/devices/Voice_state.h>

#include <stdint.h>


Device_state_create_func new_Filter_pstate;

Voice_state_get_size_func Filter_vstate_get_size;
Voice_state_init_func Filter_vstate_init;
Voice_state_render_voice_func Filter_vstate_render_voice;


#endif // KQT_FILTER_STATE_H


