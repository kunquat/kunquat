

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PARSE_MANAGER_H
#define K_PARSE_MANAGER_H


#include <stdbool.h>

#include <Handle_private.h>


/**
 * Parses data based on the given key.
 *
 * If the key does not affect the player, this function always succeeds.
 * Otherwise it will parse the data and, if the data is valid, it will update
 * the player state.
 *
 * \param handle   The Kunquat Handle -- must be valid and must support
 *                 writing.
 * \param key      The key of the data -- must not be \c NULL.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length -- must be >= \c 0.
 *
 * \return   \c true if the key is valid or not player-specific, otherwise
 *           \c false.
 */
bool parse_data(kqt_Handle* handle,
                const char* key,
                void* data,
                long length);


#endif // K_PARSE_MANAGER_H


