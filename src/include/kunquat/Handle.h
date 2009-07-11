

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


#ifndef KQT_HANDLE_H
#define KQT_HANDLE_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/limits.h>
#include <kunquat/frame.h>


typedef struct kqt_Handle kqt_Handle;


/**
 * Creates a new Kunquat Handle.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *
 * \return   The new Kunquat Handle if successful, or \c NULL if memory
 *           allocation failed.
 */
kqt_Handle* kqt_new_Handle(long buffer_size);


/**
 * Gets error information from the Kunquat Handle.
 *
 * \param handle   The Handle, or \c NULL if retrieving error information
 *                  that is not associated with a Kunquat Handle.
 *
 * \return   The error message. This is an empty string if no error has
 *           occurred.
 */
char* kqt_Handle_get_error(kqt_Handle* handle);


/**
 * Loads contents of a Kunquat composition file or directory.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param path      The path to the Kunquat composition file or directory
 *                  -- should not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
int kqt_Handle_load(kqt_Handle* handle, char* path);


/**
 * Gets the length of a Subsong in the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param subsong   The Subsong number -- should be >= \c 0 and
 *                  < \c SUBSONGS_MAX.
 *
 * \return   The length of the Subsong, or \c -1 if arguments were invalid.
 */
int kqt_Handle_get_subsong_length(kqt_Handle* handle, int subsong);


/**
 * Destroys an existing Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 */
void kqt_del_Handle(kqt_Handle* handle);


#ifdef __cplusplus
}
#endif


#endif // KQT_HANDLE_H


