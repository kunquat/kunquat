

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


#ifndef KQT_DELAY_STATE_H
#define KQT_DELAY_STATE_H


#include <init/devices/Device_impl.h>
#include <player/devices/Device_state.h>
#include <player/devices/Proc_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>


Device_impl_get_port_groups_func Delay_get_port_groups;

Device_state_create_func new_Delay_pstate;

bool Delay_pstate_set_max_delay(
        Device_state* dstate, const Key_indices indices, double value);


#endif // KQT_DELAY_STATE_H


