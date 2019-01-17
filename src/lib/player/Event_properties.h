

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


/**
 * Get the parameter type of a given Event type.
 *
 * \param event_type   The Event type -- must be valid.
 *
 * \return   The parameter type.
 */
Value_type Event_properties_get_param_type(Event_type event_type);


/**
 * Get the parameter validator for a given Event type.
 *
 * \param event_type   The Event type -- must be valid.
 *
 * \return   The parameter type, or \c NULL.
 */
Param_validator* Event_properties_get_param_validator(Event_type event_type);


/**
 * Get the name setter event associated with a given Event type.
 *
 * \param event_type   The Event type -- must be valid.
 *
 * \return   The name of the setter event, or an empty string if not applicable.
 */
const char* Event_properties_get_name_event(Event_type event_type);


#endif // KQT_EVENT_PROPERTIES_H


