

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
 *
 * Every Kunquat composition is accessed through a Kunquat Handle which has
 * the type kqt_Handle. The user needs to create a Kunquat Handle first, then
 * operate the Handle in order to play and/or modify the composition, and
 * finally release the allocated resources by destroying the Handle.
 *
 * An API for simple playback functionality is located in \c Player.h.
 */


/**
 * The identifier for accessing a single Kunquat composition.
 *
 * All functions that operate on Kunquat Handles may set an error message
 * inside the Handle. See kqt_Handle_get_error for more information.
 *
 * Operations on Kunquat Handles are generally <b>not</b> thread-safe. In
 * particular, multiple threads must not create new Kunquat Handles or access
 * a single Kunquat Handle in parallel. However, accessing different Kunquat
 * Handles from different threads in parallel should be safe.
 */
typedef struct kqt_Handle kqt_Handle;


/**
 * The access mode of Kunquat Handles.
 *
 * The read-only mode is used for composition files and is typically used by
 * players.
 *
 * The read/write mode 
 *
 * The commit mode is similar to read/write mode but provides a safe
 * transaction mechanism for storing changes to the data being modified.
 */
typedef enum
{
    KQT_READ = 0,
    KQT_READ_WRITE,
    KQT_READ_WRITE_COMMIT
} kqt_Access_mode;


/**
 * Creates a read-only Kunquat Handle from a composition file.
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
 * \param path          The path to the Kunquat composition file -- should not
 *                      be \c NULL.
 *
 * \return   The new read-only Kunquat Handle if successful, otherwise \c NULL
 *           (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_r(long buffer_size, char* path);


/**
 * Creates a read/write Kunquat Handle from a composition directory.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *                      See kqt_new_Handle_r for detailed explanation.
 * \param path          The path to the Kunquat composition directory --
 *                      should not be \c NULL.
 *
 * \return   The new read/write Kunquat Handle if successful, otherwise \c NULL
 *           (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_rw(long buffer_size, char* path);


/**
 * Gets data from the Kunquat Handle associated with the given key.
 *
 * \param handle   The Kunquat Handle -- should not be \c NULL.
 * \param key      The key of the data -- should not be \c NULL.
 *
 * \return   The data if existent and no error occurred, otherwise \c NULL.
 *           Check kqt_Handle_error(handle) for errors. The caller should
 *           eventually free the returned buffer.
 */
void* kqt_Handle_get_data(kqt_Handle* handle, const char* key);


/**
 * Gets length of data from the Kunquat Handle associated with the given key.
 *
 * \param handle   The Kunquat Handle -- should not be \c NULL.
 * \param key      The key of the data -- should not be \c NULL.
 *
 * \return   The length of the data if successful. Otherwise, \c -1 is
 *           returned and Kunquat Handle error is set accordingly.
 */
long kqt_Handle_get_data_length(kqt_Handle* handle, const char* key);


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
 * Clears error information from the Kunquat Handle.
 *
 * \param handle   The Handle, or \c NULL if the generic error message should
 *                 be cleared.
 */
void kqt_Handle_clear_error(kqt_Handle* handle);


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


