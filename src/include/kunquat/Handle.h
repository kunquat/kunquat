

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_HANDLE_H
#define KQT_HANDLE_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/limits.h>


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
 * The header \c Player.h contains an API for simple playback functionality.
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
 * \param path   The path to the Kunquat composition file -- should not
 *               be \c NULL.
 *
 * \return   The new read-only Kunquat Handle if successful, otherwise \c NULL
 *           (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_r(char* path);


/**
 * Creates a read/write Kunquat Handle from a composition directory.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \param path   The path to the Kunquat composition directory --
 *               should not be \c NULL.
 *
 * \return   The new read/write Kunquat Handle if successful, otherwise \c NULL
 *           (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_rw(char* path);


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
 * Sets data of the Kunquat Handle associated with the given key.
 *
 * \param handle   The Kunquat Handle -- should be valid and should support
 *                 writing.
 * \param key      The key of the data -- should not be \c NULL.
 * \param data     The data to be set -- should not be \c NULL unless
 *                 \a length is \c 0.
 * \param length   The length of \ə data -- must not exceed the real length.
 *
 * \return   \c 1 if successful. Otherwise, \c 0 is returned and the Kunquat
 *           Handle error is set accordingly.
 */
int kqt_Handle_set_data(kqt_Handle* handle,
                        char* key,
                        void* data,
                        int length);


/**
 * Gets an error message from the Kunquat Handle.
 *
 * An error message consists of an error code followed by a colon and an
 * error description. Possible error codes are:
 *
 * \li ArgumentError -- a Kunquat function was called with an inappropriate
 *                      argument value.
 * \li FormatError   -- an input file or value to be stored was invalid.
 * \li MemoryError   -- memory allocation failed.
 * \li ResourceError -- libkunquat couldn't get service from an external
 *                      resource.
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


