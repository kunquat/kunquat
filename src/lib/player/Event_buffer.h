

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_BUFFER_H
#define KQT_EVENT_BUFFER_H


#include <kunquat/limits.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Event_buffer Event_buffer;


/**
 * Maximum size of an Event buffer.
 */
#define EVENT_BUF_SIZE_MAX 262144


/**
 * Maximum number of bytes reserved for a single event.
 */
#define EVENT_LEN_MAX 256


/**
 * Create a new Event buffer.
 *
 * \param size   The size of the Event buffer in bytes -- must be > \c 0 and
 *               <= \c EVENT_BUF_SIZE_MAX. If <= \c EVENT_LEN_MAX + 1, the
 *               buffer cannot contain any events.
 *
 * \return   The new Event buffer if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_buffer* new_Event_buffer(int32_t size);


/**
 * Tell whether the Event buffer is empty.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 *
 * \return   \c true if the Event buffer is empty, otherwise \c false.
 */
bool Event_buffer_is_empty(const Event_buffer* ebuf);


/**
 * Tell whether the Event buffer is full.
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
 * Reset counter of added events to 0.
 *
 * This function should be called each time an event has been fully processed.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL and must not be in
 *               skipping mode.
 */
void Event_buffer_reset_add_counter(Event_buffer* ebuf);


/**
 * Start event skipping.
 *
 * Events that were fired on a suspended render call must be skipped when
 * processing the remaining events of a bind procedure.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL and must not be in
 *               skipping mode.
 */
void Event_buffer_start_skipping(Event_buffer* ebuf);


/**
 * Return the event skipping status.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 *
 * \return   \c true if the next added event will be skipped,
 *           otherwise \c false.
 */
bool Event_buffer_is_skipping(const Event_buffer* ebuf);


/**
 * Get the Event buffer contents.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 *
 * \return   The events.
 */
const char* Event_buffer_get_events(const Event_buffer* ebuf);


/**
 * Add an event to the Event buffer.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL and must not be full.
 * \param ch     The channel number -- must be >= \c 0 and
 *               < \c KQT_CHANNELS_MAX.
 * \param name   The event name -- must not be \c NULL.
 * \param arg    The event argument -- must not be \c NULL.
 */
void Event_buffer_add(Event_buffer* ebuf, int ch, const char* name, const Value* arg);


/**
 * Clear the Event buffer.
 *
 * \param ebuf   The Event buffer -- must not be \c NULL.
 */
void Event_buffer_clear(Event_buffer* ebuf);


/**
 * Destroy the Event buffer.
 *
 * \param ebuf   The Event buffer, or \c NULL.
 */
void del_Event_buffer(Event_buffer* ebuf);


#endif // KQT_EVENT_BUFFER_H


