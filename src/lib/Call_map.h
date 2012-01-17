

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


#ifndef K_CALL_MAP_H
#define K_CALL_MAP_H


#include <Event_names.h>
#include <File_base.h>
#include <kunquat/limits.h>


/**
 * A collection of conditions for automatic call and signal events.
 */
typedef struct Call_map Call_map;


/**
 * Creates a new Call map from a JSON string.
 *
 * \param str     The JSON string, or \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Call map if successful, otherwise \c NULL. \a state
 *           will not be modified if memory allocation failed.
 */
Call_map* new_Call_map(char* str,
                       Event_names* names,
                       Read_state* state);


/**
 * Resets the Call map.
 *
 * \param map   The Call map -- must not be \c NULL.
 */
void Call_map_reset(Call_map* map);


/**
 * Gets the first (call or signal) event that is a result from call mapping.
 *
 * \param map          The Call map -- must not be \c NULL.
 * \param src_event    The fired event -- must not be \c NULL.
 * \param dest_event   The storage location for the destination event --
 *                     must not be \c NULL.
 * \param dest_size    The amount of bytes reserved for \a dest_event --
 *                     must be positive. A size of at least 65 bytes is
 *                     recommended. JSON strings longer than \a dest_size - 1
 *                     bytes are truncated and thus may be invalid.
 *
 * \return   \c true if an event was stored to \a dest_event,
 *           otherwise \c false.
 */
bool Call_map_get_first(Call_map* map,
                        char* src_event,
                        char* dest_event,
                        int dest_size);


/**
 * Gets the next event mapped from a previously given event.
 *
 * \param map          The Call map -- must not be \c NULL.
 * \param dest_event   The storage location for the destination event --
 *                     must not be \c NULL.
 * \param dest_size    The amount of bytes reserved for \a dest_event --
 *                     must be positive. A size of at least 65 bytes is
 *                     recommended. JSON strings longer than \a dest_size - 1
 *                     bytes are truncated and thus may be invalid.
 *
 * \return   \c true if an event was stored, otherwise \c false.
 */
bool Call_map_get_next(Call_map* map,
                       char* dest_event,
                       int dest_size);


/**
 * Destroys an existing Call map.
 *
 * \param map   The Call map, or \c NULL.
 */
void del_Call_map(Call_map* map);


#endif // K_CALL_MAP_H


