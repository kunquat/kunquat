

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


#ifndef K_SET_MAP_H
#define K_SET_MAP_H


#include <Event_names.h>
#include <File_base.h>


/**
 * A collection of mappings from environment variable setters.
 */
typedef struct Set_map Set_map;


/**
 * Creates a new Set map from a JSON string.
 *
 * \param str     The JSON string, or \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Set map if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Set_map* new_Set_map_from_string(char* str,
                                 Event_names* names,
                                 Read_state* state);


/**
 * Destroys an existing Set map.
 *
 * \param map   The Set map, or \c NULL.
 */
void del_Set_map(Set_map* map);


#endif // K_SET_MAP_H


