

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_KEY_PATTERN_H
#define KQT_KEY_PATTERN_H


#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define KEY_INDICES_MAX 16
#define KEY_INDEX_DIGITS_MAX 6


typedef int32_t Key_indices[KEY_INDICES_MAX];


/**
 * Extract a key pattern with component indices.
 *
 * \param key           The key -- must not be \c NULL and must be shorter than
 *                      \c KQT_KEY_LENGTH_MAX (excluding '\0').
 * \param key_pattern   The storage location for the key pattern -- must not
 *                      be \c NULL and must contain space for at least
 *                      \c KQT_KEY_LENGTH_MAX bytes.
 * \param indices       The storage location for the key indices -- must not
 *                      be \c NULL and must contain space for at least
 *                      \c KEY_INDICES_MAX indices.
 *
 * \return   \c true if successful, or \c false if a pattern could not be
 *           extracted from \a key.
 */
bool extract_key_pattern(const char* key, char* key_pattern, Key_indices indices);


#endif // KQT_KEY_PATTERN_H


