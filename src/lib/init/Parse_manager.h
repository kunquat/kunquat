

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PARSE_MANAGER_H
#define KQT_PARSE_MANAGER_H


#include <Handle_private.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * Parse data based on the given key.
 *
 * If the key does not affect the player, this function always succeeds.
 * Otherwise it will parse the data and, if the data is valid, it will update
 * the player state.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL.
 * \param key      The key of the data -- must not be \c NULL.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length -- must be >= \c 0.
 *
 * \return   \c true if the key is valid or not player-specific, otherwise
 *           \c false.
 */
bool parse_data(Handle* handle, const char* key, const void* data, long length);


#endif // KQT_PARSE_MANAGER_H


