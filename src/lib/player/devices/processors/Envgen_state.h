

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


#ifndef KQT_ENVGEN_STATE_H
#define KQT_ENVGEN_STATE_H


#include <player/devices/Voice_state.h>


Voice_state_get_size_func Envgen_vstate_get_size;
Voice_state_init_func Envgen_vstate_init;
Voice_state_render_voice_func Envgen_vstate_render_voice;
Voice_state_fire_event_func Envgen_vstate_fire_event;


#endif // KQT_ENVGEN_STATE_H


