

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


