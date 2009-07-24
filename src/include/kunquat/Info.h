

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


#ifndef KQT_INFO_H
#define KQT_INFO_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>
#include <kunquat/limits.h>


/**
 * Gets the length of a Subsong in the Kunquat Handle.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param subsong   The Subsong number -- should be >= \c -1 and
 *                  < \c KQT_SUBSONGS_MAX. Using \c -1 will return the total
 *                  number of sections in all Subsongs.
 *
 * \return   The length of the Subsong, or \c -1 if arguments were invalid.
 */
int kqt_Handle_get_subsong_length(kqt_Handle* handle, int subsong);


#ifdef __cplusplus
}
#endif


#endif // KQT_INFO_H


