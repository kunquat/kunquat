

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_BIND_H
#define K_BIND_H


#include <Event_cache.h>
#include <Event_names.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <player/Env_state.h>
#include <Random.h>
#include <Value.h>


/**
 * A collection of conditions for automatic events.
 */
typedef struct Bind Bind;


/**
 * A list node returned by the Call map.
 */
typedef struct Target_event
{
    int ch_offset;
    char* desc;
    struct Target_event* next;
} Target_event;


/**
 * Creates a new Bind from a JSON string.
 *
 * \param str     The JSON string, or \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Bind if successful, otherwise \c NULL. \a state will not
 *           be modified if memory allocation failed.
 */
Bind* new_Bind(char* str, const Event_names* names, Read_state* state);


/**
 * Creates an Event cache for the Bind.
 *
 * \param map   The Bind -- must not be \c NULL.
 *
 * \return   The new Event cache if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_cache* Bind_create_cache(const Bind* map);


/**
 * Gets the first event that is a result from binding.
 *
 * \param map          The Bind -- must not be \c NULL.
 * \param cache        The Event cache -- must not be \c NULL.
 * \param estate       The Environment state -- must not be \c NULL.
 * \param event_name   The name of the fired event -- must not be \c NULL.
 * \param value        The event parameter -- must not be \c NULL.
 * \param rand         The random source -- must not be \c NULL.
 *
 * \return   The first Target event if any calls are triggered,
 *           otherwise \c NULL.
 */
Target_event* Bind_get_first(const Bind* map,
                             Event_cache* cache,
                             Env_state* estate,
                             const char* event_name,
                             Value* value,
                             Random* rand);


/**
 * Destroys an existing Bind.
 *
 * \param map   The Bind, or \c NULL.
 */
void del_Bind(Bind* map);


#endif // K_BIND_H


