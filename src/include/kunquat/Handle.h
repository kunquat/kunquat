

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


/**
 * \defgroup Handle Handle Creation and Minimal Diagnostics
 * \{
 *
 * \brief
 * This module describes Kunquat Handle, the main identifier for accessing
 * Kunquat compositions.
 */


/**
 * The identifier for accessing a single Kunquat composition.
 *
 * All functions that operate on Kunquat Handles may set an error message
 * inside the Handle. See kqt_Handle_get_error for more information.
 *
 * Operations on Kunquat Handles are generally <b>not</b> thread-safe. In
 * particular, multiple threads should not create new Kunquat Handles or
 * access a single Kunquat Handle in parallel. However, accessing different
 * Kunquat Handles from different threads in parallel should be safe.
 */
typedef struct kqt_Handle kqt_Handle;


/**
 * Creates a new Kunquat Handle that contains an empty composition.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * The buffer size determines the maximum amount of audio data that can
 * be mixed at one time. The buffer size is given as the number of amplitude
 * values (called \a frames) for one output channel. In a typical case, the
 * calling application should set this value based on the size of its own
 * output buffers: if the application uses buffers with \a n amplitude values
 * for one output channel (e.g. in 16-bit stereo, this takes \a n * \c 4 bytes
 * in total), it should call kqt_new_Handle with a buffer size of \a n.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *
 * \return   The new Kunquat Handle if successful, otherwise \c NULL
 *           (check kqt_Handle_get_error(NULL) for error message).
 */
kqt_Handle* kqt_new_Handle(long buffer_size);


/**
 * Creates a new Kunquat Handle from a composition file or directory.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *                      See kqt_new_Handle for detailed explanation.
 * \param path          The path to the Kunquat composition file or directory
 *                      -- should not be \c NULL.
 *
 * \return   The new Kunquat Handle if successful, otherwise \c NULL
 *           (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_from_path(long buffer_size, char* path);


/**
 * Gets error information from the Kunquat Handle.
 *
 * kqt_Handle_get_error(\a handle) returns a message describing the last
 * error occurred when processing \a handle.
 *
 * kqt_Handle_get_error(\c NULL) returns a message describing the last error
 * occurred in Kunquat Handle processing in general. In a single-threaded
 * application, you can always call kqt_Handle_get_error(\c NULL) to get the
 * last error message, whether or not connected to any particular Handle.
 *
 * \param handle   The Handle, or \c NULL if retrieving error information
 *                 that is not necessarily associated with a Kunquat Handle.
 *
 * \return   The last error message. This is an empty string if no error has
 *           occurred.
 */
char* kqt_Handle_get_error(kqt_Handle* handle);


/**
 * Frees all the resources allocated for an existing Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 */
void kqt_del_Handle(kqt_Handle* handle);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_HANDLE_H


