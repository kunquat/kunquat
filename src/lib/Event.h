

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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

#include <Reltime.h>
#include <kunquat/limits.h>
#include <File_base.h>

#include <Event_names.h>
#include <Event_type.h>


/**
 * Event describes a modification of playback state.
 */
typedef struct Event
{
    Reltime pos;                   ///< The Event position.
    Event_type type;               ///< The Event type.
    Event_field_desc* field_types; ///< The field type description.
    char* desc;                    ///< Event description in JSON format.
    char* fields;                  ///< Event fields as an unparsed JSON list.
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
Event* new_Event(Event_type type, Reltime* pos);


/**
 * Creates an Event from a JSON string.
 *
 * \param str     A reference to the string -- must not be \c NULL or a
 *                pointer to \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 *
 * \return   The new Event if successful, otherwise \c NULL. \a state will
 *           not be modified if memory allocation failed.
 */
Event* new_Event_from_string(char** str, Read_state* state,
                             Event_names* names);


/**
 * Tells whether the given Event type is supported.
 *
 * \param type   The Event type -- must be valid.
 *
 * \return   \c true if \a type is supported, otherwise \c false.
 */
bool Event_type_is_supported(Event_type type);


/**
 * Parses and retrieves all fields from a string.
 *
 * \param str           The string -- must not be \c NULL.
 * \param field_descs   The field descriptions -- must not be \c NULL.
 * \param fields        The fields where the values will be stored, or
 *                      \c NULL for parsing without storage.
 * \param state         The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* Event_type_get_fields(char* str,
                            Event_field_desc field_descs[],
                            Event_field fields[],
                            Read_state* state);


/**
 * Parses an Event from a string.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param str     The textual description -- must not be \c NULL.
 * \param state   The Read state object -- must not be \c NULL.
 *
 * \return   The position in the string after the parsing. The caller must
 *           check for errors through \a state.
 */
char* Event_read(Event* event, char* str, Read_state* state);


/**
 * Gets a field type description of the Event.
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The type description -- must not be freed. The value is an array
 *           containing a field type description for each field. The array is
 *           terminated with a field type of \c EVENT_NONE. See
 *           Event_type.h for details.
 */
Event_field_desc* Event_get_field_types(Event* event);


/**
 * Gets the number of fields in the Event.
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The number of fields.
 */
int Event_get_field_count(Event* event);


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
 * Gets a JSON description of the event (does not include timestamp).
 *
 * \param event   The Event -- must not be \c NULL.
 *
 * \return   The JSON string.
 */
char* Event_get_desc(Event* event);


/**
 * Gets a textual description of all the fields of the Event.
 * 
 * \param event   The Event -- must not be \c NULL.
 */
char* Event_get_fields(Event* event);


/**
 * Destroys an existing Event.
 *
 * \param event   The Event, or \c NULL.
 */
void del_Event(Event* event);


#endif // K_EVENT_H


