

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_CONTROL_DECL_H
#define KQT_EVENT_CONTROL_DECL_H


#include <player/events/Event_interfaces.h>


// Process function declarations

#define EVENT_CONTROL_DEF(name, type_suffix, arg_type, validator) \
    Event_control_interface Event_control_##type_suffix##_process;
#include <player/events/Event_control_types.h>


#endif // KQT_EVENT_CONTROL_DECL_H


