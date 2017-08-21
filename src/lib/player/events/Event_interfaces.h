

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_INTERFACES_H
#define KQT_EVENT_INTERFACES_H


#include <decl.h>

#include <stdbool.h>


typedef bool Event_au_interface(
        const Audio_unit*,
        const Au_params*,
        Au_state*,
        Master_params*,
        Channel*,
        Device_states*,
        const Event_params*);

typedef bool Event_channel_interface(
        Channel*, Device_states*, const Master_params*, const Event_params*);

typedef bool Event_control_interface(General_state*, Channel*, const Event_params*);

typedef bool Event_general_interface(General_state*, const Event_params*);

typedef bool Event_master_interface(Master_params*, const Event_params*);


#endif // KQT_EVENT_INTERFACES_H


