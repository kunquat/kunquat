

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_CACHE_H
#define KQT_EVENT_CACHE_H


#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * A collection of event history in a Channel.
 */
typedef struct Event_cache Event_cache;


/**
 * Create a new Event cache.
 *
 * \return   The new Event cache if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_cache* new_Event_cache(void);


/**
 * Add an event to the Event cache.
 *
 * \param cache        The Event cache -- must not be \c NULL.
 * \param event_name   The name of the Event -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_cache_add_event(Event_cache* cache, char* event_name);


/**
 * Update the Event cache.
 *
 * \param cache        The Event cache -- must not be \c NULL.
 * \param event_name   The name of the Event -- must not be \c NULL.
 * \param value        The Event parameter -- must not be \c NULL.
 */
void Event_cache_update(Event_cache* cache, const char* event_name, const Value* value);


/**
 * Get a value from the Event cache.
 *
 * \param cache        The Event cache -- must not be \c NULL.
 * \param event_name   The name of the Event -- must not be \c NULL and
 *                     must be an event name found in \a cache.
 *
 * \return   The value associated with \a event_name. This is never \c NULL.
 */
const Value* Event_cache_get_value(const Event_cache* cache, const char* event_name);


/**
 * Reset the Event cache.
 *
 * \param cache        The Event cache -- must not be \c NULL.
 */
void Event_cache_reset(Event_cache* cache);


/**
 * Destroy an existing Event cache.
 *
 * \param cache   The Event cache, or \c NULL.
 */
void del_Event_cache(Event_cache* cache);


#endif // KQT_EVENT_CACHE_H


