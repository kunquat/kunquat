

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
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


#include <stdlib.h>

#include <kunquat/limits.h>
#include <Value.h>


typedef struct Event_buffer Event_buffer;


/**
 * Maximum number of bytes reserved for a single event.
 */
#define EVENT_LEN_MAX 256


/**
 * Creates a new Event buffer.
 *
 * \param size   The size of the Event buffer in bytes. If
 *               <= \c EVENT_LEN_MAX + 1, the buffer cannot contain any events.
 *
 * \return   The new Event buffer if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_buffer* new_Event_buffer(size_t size);


/**
 * Tells wheter the Event buffer is full.
 *
 * The Event buffer is considered full if it cannot store another event
 * of \c EVENT_LEN_MAX bytes.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 *
 * \return   \c true if the Event buffer is full, otherwise \c false.
 */
bool Event_buffer_is_full(const Event_buffer* ebuf);


/**
 * Gets the Event buffer contents.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 *
 * \return   The events.
 */
const char* Event_buffer_get_events(const Event_buffer* ebuf);


/**
 * Adds an event to the Event buffer.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL and must not be full.
 * \param ch     The channel number -- must be >= \c 0 and
 *               < \c KQT_CHANNELS_MAX.
 * \param name   The event name -- must not be \c NULL.
 * \param arg    The event argument -- must not be \c NULL.
 */
void Event_buffer_add(
        Event_buffer* ebuf,
        int ch,
        const char* name,
        Value* arg);


/**
 * Clears the Event buffer.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 */
void Event_buffer_clear(Event_buffer* ebuf);


/**
 * Destroys the Event buffer.
 *
 * \param ebuf   The Event buffer, or \c NULL.
 */
void del_Event_buffer(Event_buffer* ebuf);


#endif // K_EVENT_BUFFER_H


