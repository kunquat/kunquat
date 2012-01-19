

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_CACHE_H
#define K_EVENT_CACHE_H


#include <stdlib.h>
#include <stdbool.h>


/**
 * A collection of event history in a Channel.
 */
typedef struct Event_cache Event_cache;


/**
 * Creates a new Event cache.
 *
 * \return   The new Event cache if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_cache* new_Event_cache(void);


/**
 * Adds an event to the Event cache.
 *
 * \param cache        The Event cache -- must not be \c NULL.
 * \param event_name   The name of the Event -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_cache_add_event(Event_cache* cache, char* event_name);


/**
 * Resets the Event cache.
 *
 * \param cache        The Event cache -- must not be \c NULL.
 */
void Event_cache_reset(Event_cache* cache);


/**
 * Destroys an existing Event cache.
 *
 * \param cache   The Event cache, or \c NULL.
 */
void del_Event_cache(Event_cache* cache);


#endif // K_EVENT_CACHE_H


