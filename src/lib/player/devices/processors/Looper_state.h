

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_LOOPER_STATE_H
#define KQT_LOOPER_STATE_H


#include <decl.h>
#include <player/devices/Device_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>


Device_state_create_func new_Looper_pstate;

bool Looper_pstate_set_max_rec_time(
        Device_state* dstate, const Key_indices indices, double value);


#endif // KQT_LOOPER_STATE_H


