

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_NAMES_H
#define KQT_EVENT_NAMES_H


#include <decl.h>
#include <player/Event_type.h>
#include <player/Param_validator.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


//typedef struct Event_names Event_names;


/**
 * Create a new Event name collection.
 *
 * \param props   The Event properties -- must not be \c NULL.
 *
 * \return   The Event name collection if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_names* new_Event_names(Event_properties* props);


/**
 * Retrieve the Event type of the given name.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must not be \c NULL.
 *
 * \return   The Event type of the name, or EVENT_NONE if the name does not
 *           correspond to an Event type.
 */
Event_type Event_names_get(const Event_names* names, const char* name);


/**
 * Retrieve the parameter type for the given event name.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must be a supported name.
 *
 * \return   The parameter type.
 */
Value_type Event_names_get_param_type(const Event_names* names, const char* name);


/**
 * Retrieve the parameter validator for the given event name.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must be a supported name.
 *
 * \return   The parameter validator, or \c NULL if there is no validator
 *           associated with \a name.
 */
Param_validator* Event_names_get_param_validator(
        const Event_names* names, const char* name);


/**
 * Retrieve a name setting event corresponding to the given event name.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must be a supported name.
 *
 * \return   The associated name setting event, or \c NULL if not applicable
 *           with \a name.
 */
const char* Event_names_get_name_event(const Event_names* names, const char* name);


/**
 * Destroy an existing Event name collection.
 *
 * \param names   The Event name collection, or \c NULL.
 */
void del_Event_names(Event_names* names);


#endif // KQT_EVENT_NAMES_H


