

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_EVENT_H
#define K_EVENT_H


#include <stdint.h>
#include <stdbool.h>

#include <Reltime.h>
#include <Song_limits.h>

#include <Event_type.h>


/**
 * Event describes a modification of playback state.
 */
typedef struct Event
{
    Reltime pos;                   ///< The Event position.
    Event_type type;               ///< The Event type.
    Event_field_desc* field_types; ///< The field type description.
    bool (*set)(struct Event* event, int index, void* data); ///< Field setter.
    void* (*get)(struct Event* event, int index);            ///< Field getter.
    void (*destroy)(struct Event* event);                    ///< Destructor.
} Event;


/**
 * Gets a field type description of the Event.
 *
 * \param type   The Event type -- must be a valid type.
 *
 * \return   The type description -- must not be freed. The value is an array
 *           containing a field type description for each field. The array is
 *           terminated with a field type of \c EVENT_TYPE_NONE. See
 *           Event_type.h for details.
 */
Event_field_desc* Event_get_field_types(Event* event);


/**
 * Gets the Event position (relative to the containing Pattern).
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The position.
 */
Reltime* Event_get_pos(Event* event);


/**
 * Moves the Event into a new position.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param pos     The new position -- must not be \c NULL.
 */
void Event_set_pos(Event* event, Reltime* pos);


/**
 * Gets the type of an Event.
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The Event type.
 */
Event_type Event_get_type(Event* event);


/**
 * Sets a field in the Event.
 *
 * If the given index is valid, the type and constraints of the field can
 * be found in Event_get_field_types(event)[index].
 *
 * \param event   The Event -- must not be \c NULL.
 * \param index   The index.
 * \param data    A pointer to the value -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if the index or value was
 *           illegal.
 */
bool Event_set_field(Event* event, int index, void* data);


/**
 * Retrieves a field from the Event.
 * 
 * \param event   The Event -- must not be \c NULL.
 * \param index   The index.
 *
 * \return   A pointer to the field if one exists, otherwise \c NULL.
 */
void* Event_get_field(Event* event, int index);


/**
 * Destroys an existing Event.
 *
 * \param event   The Event -- must not be \c NULL.
 */
void del_Event(Event* event);


#endif // K_EVENT_H


