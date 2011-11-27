

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_BUFFER_H
#define K_EVENT_BUFFER_H


#include <stdbool.h>


/**
 * A buffer containing outgoing events.
 */
typedef struct Event_buffer Event_buffer;


/**
 * Creates a new Event buffer.
 *
 * \param size   Buffer size in bytes -- must be > \c 0 and
 *               < \c 32768.
 *
 * \return   The new Event buffer if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_buffer* new_Event_buffer(int size);


/**
 * Adds an event to the Event buffer.
 *
 * \param buf     The Event buffer -- must not be \c NULL.
 * \param event   The event description -- must not be \c NULL and
 *                must fit inside the buffer.
 *
 * \return   \c true if unread events were dropped off the buffer,
 *           otherwise \c false.
 */
bool Event_buffer_add(Event_buffer* buf, char* event);


/**
 * Gets an event from the Event buffer.
 *
 * \param buf    The Event buffer -- must not be \c NULL.
 * \param dest   The destination buffer, or \c NULL if skipping.
 * \param size   The size of the destination buffer including the
 *               terminating byte -- must be positive unless
 *               \a dest is \c NULL.
 *
 * \return   \c true if an event description was found, otherwise \c false.
 */
bool Event_buffer_get(Event_buffer* buf, char* dest, int size);


/**
 * Clears the Event buffer.
 *
 * \param buf   The Event buffer -- must not be \c NULL.
 */
void Event_buffer_clear(Event_buffer* buf);


/**
 * Destroys an existing Event buffer.
 *
 * \param buf   The Event buffer, or \c NULL.
 */
void del_Event_buffer(Event_buffer* buf);


#endif // K_EVENT_BUFFER_H


