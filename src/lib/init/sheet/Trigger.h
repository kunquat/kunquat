

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_TRIGGER_H
#define KQT_TRIGGER_H


#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * Trigger causes firing of an event at a specified location.
 */
typedef struct Trigger
{
    Tstamp pos;         ///< The Trigger position.
    int ch_index;       ///< Channel number.
    Event_type type;    ///< The event type.
    char* desc;         ///< Trigger description in JSON format.
} Trigger;


/**
 * Check for name specifier in the trigger description.
 *
 * \params sr   The Streader of the data -- must not be \c NULL. The function
 *              does not modify the position of \a sr but may set an error.
 *
 * \return   \c true if the trigger description contains a name specification
 *           of the form :name, otherwise \c false.
 */
bool Trigger_data_contains_name_spec(Streader* sr);


/**
 * Create a Trigger of specified type.
 *
 * \param type   The event type -- must be valid.
 * \param pos    The Trigger position -- must not be \c NULL.
 *
 * \return   The new Trigger if successful, or \c NULL if memory allocation
 *           failed or the event type isn't supported.
 */
Trigger* new_Trigger(Event_type type, Tstamp* pos);


/**
 * Create a Trigger of name specification from a JSON string.
 *
 * \params sr     The Streader of the data -- must not be \c NULL and must be
 *                checked with \a Trigger_data_contains_name_spec beforehand.
 *                The function does not modify the position of \a sr but may
 *                set an error.
 * \param names   The Event names -- must not be \c NULL.
 *
 * \return   The new Trigger if successful, otherwise \c NULL.
 */
Trigger* new_Trigger_of_name_spec_from_string(Streader* sr, const Event_names* names);


/**
 * Create a Trigger from a JSON string.
 *
 * \param sr      The Streader of the data -- must not be \c NULL.
 *                NOTE: The function will modify the Streader, so
 *                \a new_Trigger_of_name_spec_from_string must be called first
 *                if needed.
 * \param names   The Event names -- must not be \c NULL.
 *
 * \return   The new Trigger if successful, otherwise \c NULL.
 */
Trigger* new_Trigger_from_string(Streader* sr, const Event_names* names);


/**
 * Get the Trigger position (relative to the containing Pattern).
 *
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   The position.
 */
const Tstamp* Trigger_get_pos(const Trigger* trigger);


/**
 * Get the event type of the Trigger.
 *
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   The event type.
 */
Event_type Trigger_get_type(const Trigger* trigger);


/**
 * Get a JSON description of the Trigger (does not include timestamp).
 *
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   The JSON string.
 */
const char* Trigger_get_desc(const Trigger* trigger);


/**
 * Destroy an existing Trigger.
 *
 * \param trigger   The Trigger, or \c NULL.
 */
void del_Trigger(Trigger* trigger);


#endif // KQT_TRIGGER_H


