

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


#ifndef KQT_RANGEMAP_STATE_H
#define KQT_RANGEMAP_STATE_H


#include <player/devices/Device_state.h>
#include <player/devices/Voice_state.h>

#include <stdint.h>


Device_state_create_func new_Rangemap_pstate;

Voice_state_init_func Rangemap_vstate_init;


#endif // KQT_RANGEMAP_STATE_H


