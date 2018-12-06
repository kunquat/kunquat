

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEBUG_STATE_H
#define KQT_DEBUG_STATE_H


#include <init/devices/Device_impl.h>
#include <player/devices/Voice_state.h>


Device_impl_get_port_groups_func Debug_get_port_groups;
Voice_state_init_func Debug_vstate_init;
Voice_state_render_voice_func Debug_vstate_render_voice;


#endif // KQT_DEBUG_STATE_H


