

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


typedef enum
{
    /// An uninitialised event.
    EVENT_TYPE_NONE = 0,
    /// Evaluate a conditional expression.
    EVENT_TYPE_GENERAL_COND,
    /// Sentinel -- never used as a valid type.
    EVENT_TYPE_GENERAL_LAST = 63,
    /// Set a variable.
    EVENT_TYPE_GLOBAL_SET_VAR,
    /// Set tempo.
    /// Fields: BPM (float)
    EVENT_TYPE_GLOBAL_TEMPO,
    /// Set global volume.
    EVENT_TYPE_GLOBAL_VOLUME,
    /// Sentinel -- never used as a valid type.
    EVENT_TYPE_GLOBAL_LAST = 127,
    /// Note On event.
    /// Fields: note, modifier (optional), octave, instrument
    EVENT_TYPE_NOTE_ON,
    /// Note Off event.
    /// Fields: none
    EVENT_TYPE_NOTE_OFF,
    /// Sentinel -- never used as a valid type.
    EVENT_TYPE_LAST
} Event_type;


#define EVENT_TYPE_IS_GENERAL(type) ((type) > EVENT_TYPE_NONE && (type) < EVENT_TYPE_GENERAL_LAST)
#define EVENT_TYPE_IS_GLOBAL(type)  ((type) > EVENT_TYPE_GENERAL_LAST && (type) < EVENT_TYPE_GLOBAL_LAST)
#define EVENT_TYPE_IS_INS(type)     ((type) > EVENT_TYPE_GLOBAL_LAST && (type) < EVENT_TYPE_LAST)
#define EVENT_TYPE_IS_VALID(type)   (EVENT_TYPE_IS_GENERAL(type) || EVENT_TYPE_IS_GLOBAL(type) || EVENT_TYPE_IS_INS(type))


/**
 * Gets a field type description of the given Event type.
 *
 * \param type   The Event type -- must be a valid type.
 *
 * \return   The type description -- must not be freed. The value is a string
 *           with a character describing the type of each field. Two types are
 *           supported: 'i' for 64-bit signed integer, and 'f' for
 *           double-precision floating point number. E.g. "iif" means that the
 *           Event contains three fields -- the first two are integers and the
 *           last one is float. \c NULL means that the given Event type is not
 *           in use.
 */
char* Event_type_get_field_types(Event_type type);


/**
 * Event describes a modification of playback state.
 */
typedef struct Event
{
    /// The Event position.
    Reltime pos;
    /// The Event type.
    Event_type type;
    /// The data fields.
    union
    {
        int64_t i;
        double d;
    } fields[EVENT_FIELDS];
} Event;


/**
 * Creates a new Event.
 *
 * \param pos    The position of the Event -- must not be \c NULL.
 * \param type   The Event type -- must be a valid type.
 *
 * \return   The new Event if successful, or \c NULL if memory allocation
 *           failed.
 */
Event* new_Event(Reltime* pos, Event_type type);


/**
 * Resets an Event and changes its type. The position will not be altered.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param type    The new Event type -- must be a valid type.
 */
void Event_reset(Event* event, Event_type type);


/**
 * Gets the Event position (relative to the containing Pattern).
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The position.
 */
Reltime* Event_pos(Event* event);


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
 * Retrieves a field interpreted as an integer from the Event.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param index   The index -- must be < \c EVENT_FIELDS.
 * \param value   The storage location of the read value -- must not be
 *                \c NULL. If the read fails, the memory area won't be
 *                changed.
 *
 * \return   \c true if a valid value was read, otherwise \c false.
 */
bool Event_int(Event* event, uint8_t index, int64_t* value);


/**
 * Retrieves a field interpreted as a floating point value from the Event.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param index   The index -- must be < \c EVENT_FIELDS.
 * \param value   The storage location of the read value -- must not be
 *                \c NULL. If the read fails, the memory area won't be
 *                changed.
 *
 * \return   \c true if a valid value was read, otherwise \c false.
 */
bool Event_float(Event* event, uint8_t index, double* value);


/**
 * Sets a field as an integer.
 *
 * The value must satisfy the requirements of the Event type.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param index   The index -- must be < \c EVENT_FIELDS.
 * \param value   The value to be set.
 *
 * \return   \c true if a value was correctly set, otherwise \c false.
 */
bool Event_set_int(Event* event, uint8_t index, int64_t value);


/**
 * Sets a field as a float.
 *
 * The value must satisfy the requirements of the Event type.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param index   The index -- must be < \c EVENT_FIELDS.
 * \param value   The value to be set.
 *
 * \return   \c true if a value was correctly set, otherwise \c false.
 */
bool Event_set_float(Event* event, uint8_t index, double value);


/**
 * Destroys an existing Event.
 *
 * \param event   The Event -- must not be \c NULL.
 */
void del_Event(Event* event);


#endif // K_EVENT_H


