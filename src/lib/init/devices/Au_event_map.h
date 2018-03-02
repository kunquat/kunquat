

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

#include <stdlib.h>


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


