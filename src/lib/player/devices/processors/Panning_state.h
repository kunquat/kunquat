

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PANNING_STATE_H
#define KQT_PANNING_STATE_H


#include <init/devices/Device_impl.h>
#include <player/devices/Device_state.h>
#include <player/devices/Voice_state.h>


Device_impl_get_port_groups_func Panning_get_port_groups;

Device_state_create_func new_Panning_pstate;
Set_state_float_func Panning_pstate_set_panning;

Voice_state_get_size_func Panning_vstate_get_size;
Voice_state_render_voice_func Panning_vstate_render_voice;


#endif // KQT_PANNING_STATE_H


