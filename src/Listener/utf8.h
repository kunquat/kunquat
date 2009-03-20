

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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


#ifndef K_UTF8_H
#define K_UTF8_H


#include <wchar.h>
#include <errno.h>


/**
 * Translates a UTF-8 string into UCS-4.
 *
 * \param dest   The destination buffer -- must not be \c NULL. The buffer
 *               must have space for at least \a len elements.
 * \param src    The source buffer -- must not be \c NULL.
 * \param len    The maximum number of characters to be written into \a dest
 *               -- includes the terminating L'\0'.
 *
 * \return   \c 0 on success, \c EILSEQ if \a src is malformed.
 */
int from_utf8(wchar_t* dest, unsigned char* src, int len);


/**
 * Translates a UCS-4 string into UTF-8.
 *
 * \param dest   The destination buffer -- must not be \c NULL. The buffer
 *               must have space for at least \a len elements.
 * \param src    The source buffer -- must not be \c NULL.
 * \param len    The maximum number of characters to be written into \a dest
 *               -- includes the terminating '\0'.
 *
 * \return   \c 0 on success, \c EILSEQ if \a src is malformed.
 */
int to_utf8(unsigned char* dest, wchar_t* src, int len);


#endif // K_UTF8_H


