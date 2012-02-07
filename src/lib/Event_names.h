

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_NAMES_H
#define K_EVENT_NAMES_H


#include <stdbool.h>

#include <Event_type.h>


#define EVENT_NAME_MAX 12


typedef struct Event_names Event_names;


/**
 * Creates a new Event name collection.
 *
 * \return   The Event name collection if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_names* new_Event_names(void);


/**
 * Adds an Event name into the Event name collection.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must not be \c NULL, empty string or
 *                longer than EVENT_NAME_MAX characters. Also, the name must
 *                not already exist in the collection.
 * \param type    The Event type -- must be a valid supported type.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
//bool Event_names_add(Event_names* names, const char* name, Event_type type);


/**
 * Finds out whether a fatal error has occurred when adding Event names.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 *
 * \return   \c true if an error has occurred, otherwise \c false.
 */
bool Event_names_error(Event_names* names);


/**
 * Retrieves the Event type of the given name.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must not be \c NULL.
 *
 * \return   The Event type of the name, or EVENT_NONE if the name does not
 *           correspond to an Event type.
 */
Event_type Event_names_get(Event_names* names, const char* name);


/**
 * Retrieves the parameter type for the given event name.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must be a supported name.
 *
 * \return   The parameter type.
 */
Event_field_type Event_names_get_param_type(Event_names* names,
                                            const char* name);


/**
 * Sets filter passing status for an event.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must not be \c NULL.
 * \param pass    \c true if and only if the events of \a name should pass
 *                the event filter.
 *
 * \return   \c true if successful, or \c false if \a name does not correspond
 *           to an Event type.
 */
bool Event_names_set_pass(Event_names* names, const char* name, bool pass);


/**
 * Gets filter passing status for an event.
 *
 * \param names   The Event name collection -- must not be \c NULL.
 * \param name    The Event name -- must not be \c NULL.
 *
 * \return   \c true if \a name is valid and it passes the filter,
 *           otherwise \c false.
 */
bool Event_names_get_pass(Event_names* names, const char* name);


/**
 * Destroys an existing Event name collection.
 *
 * \param names   The Event name collection, or \c NULL.
 */
void del_Event_names(Event_names* names);


#endif // K_EVENT_NAMES_H


