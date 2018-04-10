

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_AU_EVENT_MAP
#define KQT_AU_EVENT_MAP


#include <decl.h>
#include <Value.h>

#include <stdlib.h>


typedef enum
{
    AU_EVENT_TARGET_DEV_AU,
    AU_EVENT_TARGET_DEV_PROC,
} Au_event_target_dev_type;


typedef struct Au_event_iter_result
{
    Au_event_target_dev_type dev_type;
    int dev_index;
    const char* event_name;
    Value arg;
} Au_event_iter_result;


typedef struct Au_event_bind_entry Au_event_bind_entry;

typedef struct Au_event_iter
{
    Au_event_iter_result result;
    const Au_event_bind_entry* bind_entry;
    Value src_value;
    Random* rand;
} Au_event_iter;

#define AU_EVENT_ITER_AUTO (&(Au_event_iter){ .bind_entry = NULL, .rand = NULL })


/**
 * Initialise Audio unit event iterator.
 *
 * \param iter         The iterator -- must not be \c NULL.
 * \param map          The Audio unit event map -- must not be \c NULL.
 * \param event_name   The name of the event -- must not be \c NULL.
 * \param arg          The argument -- must not be \c NULL.
 * \param rand         The Random source -- must not be \c NULL.
 *
 * \return   The first iteration result if one exists, otherwise \c NULL.
 */
Au_event_iter_result* Au_event_iter_init(
        Au_event_iter* iter,
        const Au_event_map* map,
        const char* event_name,
        const Value* arg,
        Random* rand);


/**
 * Get next iteration result from the Audio unit event iterator.
 *
 * \param iter   The iterator -- must not be \c NULL.
 *
 * \return   The next iteration result if one exists, otherwise \c NULL.
 */
Au_event_iter_result* Au_event_iter_get_next(Au_event_iter* iter);


/**
 * Create new Audio unit event map.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Audio unit event map if successful, otherwise \c NULL.
 */
Au_event_map* new_Au_event_map(Streader* sr);


/**
 * Destroy an existing Audio unit event map.
 *
 * \param map   The Audio unit event map, or \c NULL.
 */
void del_Au_event_map(Au_event_map* map);


#endif // KQT_AU_EVENT_MAP


