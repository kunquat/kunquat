

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
 * Creates an empty write-only Kunquat Handle that only resides in memory.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \return   The new memory-only Kunquat Handle if successful, otherwise
 *           \c NULL (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_m(void);


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
                        const char* key,
                        void* data,
                        long length);


/**
 * Gets an error message from the Kunquat Handle.
 *
 * An error message is a JSON object string that contains at least two keys:
 * "type" and "message". The value of "type" is one of the following:
 *
 * \li "ArgumentError" -- a Kunquat function was called with an inappropriate
 *                      argument value.
 * \li "FormatError"   -- an input file or value to be stored was invalid.
 * \li "MemoryError"   -- memory allocation failed.
 * \li "ResourceError" -- libkunquat couldn't get service from an external
 *                      resource.
 *
 * The value of "message" is a human-readable description of the error.
 *
 * kqt_Handle_get_error(\a handle) returns a JSON object describing the last
 * error occurred when processing \a handle.
 *
 * kqt_Handle_get_error(\c NULL) returns a JSON object describing the last
 * error occurred in Kunquat Handle processing in general. In a
 * single-threaded application, you can always call
 * kqt_Handle_get_error(\c NULL) to get the last error message, whether or
 * not related to any particular Handle.
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
 * NOTE: This function also frees the memory reserved for data returned by
 * calls of kqt_Handle_get_data with this Handle!
 *
 * \param handle   The Handle -- should not be \c NULL.
 */
void kqt_del_Handle(kqt_Handle* handle);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_HANDLE_H


