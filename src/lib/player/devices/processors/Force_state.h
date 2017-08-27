

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FORCE_STATE_H
#define KQT_FORCE_STATE_H


#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Force_controls.h>


Voice_state_get_size_func Force_vstate_get_size;
Voice_state_init_func Force_vstate_init;
Voice_state_render_voice_func Force_vstate_render_voice;

Force_controls* Force_vstate_get_force_controls_mut(Voice_state* vstate);


#endif // KQT_FORCE_STATE_H


