

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


#include <Active_names.h>
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
 * Gets the first event mapped from a value.
 *
 * \param map          The Set map -- must not be \c NULL.
 * \param names        The active names -- must not be \c NULL.
 * \param src_event    The source (set) event -- must not be \c NULL.
 * \param dest_event   The storage location for the destination event --
 *                     must not be \c NULL.
 * \param dest_size    The amount of bytes reserved for \a dest_event --
 *                     must be positive. A size of at least 65 bytes is
 *                     recommended. JSON strings longer than \a dest_size - 1
 *                     bytes are truncated and thus may be invalid.
 *
 * \return   \c true if an event was bound and stored, otherwise \c false.
 */
bool Set_map_get_first(Set_map* map,
                       Active_names* names,
                       char* src_event,
                       char* dest_event,
                       int dest_size);


/**
 * Gets the next event mapped from a previously given value.
 *
 * \param map          The Set map -- must not be \c NULL.
 * \param dest_event   The storage location for the destination event --
 *                     must not be \c NULL.
 * \param dest_size    The amount of bytes reserved for \a dest_event --
 *                     must be positive. A size of at least 65 bytes is
 *                     recommended. JSON strings longer than \a dest_size - 1
 *                     bytes are truncated and thus may be invalid.
 *
 * \return   \c true if an event was bound and stored, otherwise \c false.
 */
bool Set_map_get_next(Set_map* map, char* dest_event, int dest_size);


/**
 * Destroys an existing Set map.
 *
 * \param map   The Set map, or \c NULL.
 */
void del_Set_map(Set_map* map);


#endif // K_SET_MAP_H


