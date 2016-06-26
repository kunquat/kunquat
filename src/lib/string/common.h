

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


#ifndef KQT_STRING_COMMON_H
#define KQT_STRING_COMMON_H


#include <stdbool.h>
#include <stdlib.h>


/**
 * Check for equality between strings.
 *
 * NULL string is considered equivalent to an empty string.
 *
 * \param str1   The first string.
 * \param str2   The second string.
 *
 * \return   \c true if and only if the strings are equal.
 */
bool string_eq(const char* str1, const char* str2);


/**
 * Check for a prefix in the given string.
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
 * Check for a suffix in the given string.
 *
 * NULL string is considered equivalent to an empty string.
 *
 * \param str      The string to be searched.
 * \param suffix   The suffix.
 *
 * \return   \c true if and only if \a suffix is a suffix of \a str.
 */
bool string_has_suffix(const char* str, const char* suffix);


/**
 * Extract an unsigned hexadecimal index from a path component.
 *
 * \param path     The path -- must not be \c NULL.
 * \param prefix   The prefix of the path, or \c NULL if irrelevant.
 * \param digits   The number of hexadecimal digits -- must be > \c 0.
 * \param after    Required characters after the digits, or \c NULL if
 *                 irrelevant. If the first character is '.', \a after
 *                 is assumed to be a file suffix and no additional
 *                 characters are accepted.
 *
 * \return   The extracted index, or \c -1 if the path is not valid.
 */
int string_extract_index(
        const char* path, const char* prefix, int digits, const char* after);


#endif // KQT_STRING_COMMON_H


