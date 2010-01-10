

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef K_PARSE_MANAGER_H
#define K_PARSE_MANAGER_H


#include <stdbool.h>

#include <Handle_private.h>


/**
 * Checks for a key component with an index.
 *
 * \param key      The key -- must not be \c NULL.
 * \param prefix   The prefix of the component -- must not be \c NULL.
 * \param digits   The number of digits after the component -- must be
 *                 > \c 0.
 *
 * \return   The number after prefix if successful, otherwise \c -1.
 */
int parse_index_dir(const char* key, const char* prefix, int digits);


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


