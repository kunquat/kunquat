

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


#ifndef K_EVENT_H
#define K_EVENT_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <Reltime.h>
#include <kunquat/limits.h>
#include <File_base.h>
#include <String_buffer.h>

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
    char* fields;
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
 * Parses and retrieves all fields from a string.
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
 * Serialises the Event.
 *
 * \param event   The Event -- must not be \c NULL.
 * \param sb      The String buffer where the Event shall be written -- must
 *                not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Event_serialise(Event* event, String_buffer* sb);


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
 * Gets a textual description of all the fields of the Event.
 * 
 * \param event   The Event -- must not be \c NULL.
 */
char* Event_get_fields(Event* event);


/**
 * Destroys an existing Event.
 *
 * \param event   The Event -- must not be \c NULL.
 */
void del_Event(Event* event);


#endif // K_EVENT_H


