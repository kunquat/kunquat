

/*
 * Copyright 2010 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
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


