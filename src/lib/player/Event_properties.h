

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_PROPERTIES_H
#define KQT_EVENT_PROPERTIES_H


#include <decl.h>
#include <player/Event_type.h>
#include <player/Param_validator.h>
#include <Value.h>

#include <stdlib.h>


Event_properties* new_Event_properties(void);


Value_type Event_properties_get_param_type(
        const Event_properties* props, Event_type event_type);


Param_validator* Event_properties_get_param_validator(
        const Event_properties* props, Event_type event_type);


const char* Event_properties_get_name_event(
        const Event_properties* props, Event_type event_type);


void del_Event_properties(Event_properties* props);


#endif // KQT_EVENT_PROPERTIES_H


