

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_STRING_COMMON_H
#define K_STRING_COMMON_H


#include <stdbool.h>


/**
 * Checks for a prefix in the given string.
 *
 * NULL string is considered equivalent to an empty string.
 *
 * \param str      The string to be searched.
 * \param prefix   The prefix.
 *
 * \return   \c true if and only if \a prefix is a prefix of \a str.
 */
bool string_has_prefix(const char* str, const char* prefix);


/**
 * Checks for a suffix in the given string.
 *
 * NULL string is considered equivalent to an empty string.
 *
 * \param str      The string to be searched.
 * \param suffix   The suffix.
 *
 * \return   \c true if and only if \a suffix is a suffix of \a str.
 */
bool string_has_suffix(const char* str, const char* suffix);


#endif // K_STRING_COMMON_H


