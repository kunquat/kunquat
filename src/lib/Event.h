

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_H
#define K_EVENT_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <kunquat/limits.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <Streader.h>
#include <Tstamp.h>


/**
 * Event describes a modification of playback state.
 */
typedef struct Event
{
    Tstamp pos;                   ///< The Event position.
    int ch_index;                  ///< Channel number.
    Event_type type;               ///< The Event type.
    char* desc;                    ///< Event description in JSON format.
    void (*destroy)(struct Event* event);                    ///< Destructor.
} Event;


/**
 * Creates an Event of specified type.
 *
 * \param type   The Event type -- must be valid.
 * \param pos    The Event position -- must not be \c NULL.
 *
 * \return   The new Event if successful, or \c NULL if memory allocation
 *           failed or the Event type isn't supported.
 */
Event* new_Event(Event_type type, Tstamp* pos);


/**
 * Creates an Event from a JSON string.
 *
 * \param sr      The Streader of the data -- must not be \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 *
 * \return   The new Event if successful, otherwise \c NULL. \a state will
 *           not be modified if memory allocation failed.
 */
Event* new_Event_from_string(Streader* sr, const Event_names* names);


/**
 * Tells whether the given Event type is supported.
 *
 * \param type   The Event type -- must be valid.
 *
 * \return   \c true if \a type is supported, otherwise \c false.
 */
//bool Event_type_is_supported(Event_type type);


/**
 * Gets the Event position (relative to the containing Pattern).
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The position.
 */
Tstamp* Event_get_pos(Event* event);


/**
 * Moves the Event into a new position.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param pos     The new position -- must not be \c NULL.
 */
void Event_set_pos(Event* event, Tstamp* pos);


/**
 * Gets the type of an Event.
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The Event type.
 */
Event_type Event_get_type(Event* event);


/**
 * Gets a JSON description of the event (does not include timestamp).
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The JSON string.
 */
char* Event_get_desc(Event* event);


/**
 * Destroys an existing Event.
 *
 * \param event   The Event, or \c NULL.
 */
void del_Event(Event* event);


#endif // K_EVENT_H


