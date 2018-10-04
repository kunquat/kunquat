

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
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
typedef int kqt_Handle;


/**
 * Create a Kunquat Handle.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \return   The new Kunquat Handle if successful, otherwise \c 0
 *           (check kqt_Handle_get_error(\c 0) for error message).
 */
kqt_Handle kqt_new_Handle(void);


/**
 * Set the number of threads used by the Kunquat Handle.
 *
 * In addition to audio rendering, multiple threads may be used by certain
 * loading operations.
 *
 * NOTE: If libkunquat is built without thread support, this function will have
 *       no effect.
 *
 * \param handle   The Handle -- should be valid.
 * \param count    The number of threads -- should be >= \c 1 and
 *                 <= \c KQT_THREADS_MAX.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Handle_set_thread_count(kqt_Handle handle, int count);


/**
 * Get the number of threads used by the Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   The number of threads used.
 */
int kqt_Handle_get_thread_count(kqt_Handle handle);


/**
 * Set data of the Kunquat Handle associated with the given key.
 *
 * After a successful call of this function, the handle is set as not
 * validated. As long as the handle is not validated, only the following
 * functions can be called successfully on the handle:
 *
 * \li kqt_Handle_set_data
 * \li kqt_Handle_get_error
 * \li kqt_Handle_clear_error
 * \li kqt_Handle_validate
 * \li kqt_del_Handle
 *
 * \param handle   The Kunquat Handle -- should be valid.
 * \param key      The key of the data -- should not be \c NULL.
 * \param data     The data to be set -- should not be \c NULL unless
 *                 \a length is \c 0.
 * \param length   The length of \a data -- must not exceed the real length.
 *
 * \return   \c 1 if successful. Otherwise, \c 0 is returned and the Kunquat
 *           Handle error is set accordingly.
 */
int kqt_Handle_set_data(
        kqt_Handle handle, const char* key, const void* data, long length);


/**
 * Get error description from the Kunquat Handle.
 *
 * An error description is a JSON object string that contains at least two
 * keys: "type" and "message". The value of "type" is one of the following:
 *
 * \li "ArgumentError" -- a Kunquat function was called with an inappropriate
 *                        argument value.
 * \li "FormatError"   -- a value to be stored was invalid.
 * \li "MemoryError"   -- memory allocation failed.
 * \li "ResourceError" -- libkunquat couldn't get service from an external
 *                        resource.
 *
 * The value of "message" is a human-readable description of the error. It can
 * also be retrieved using the function \a kqt_Handle_get_error_message.
 *
 * kqt_Handle_get_error(\a handle) returns a JSON object describing the last
 * error occurred when processing \a handle.
 *
 * kqt_Handle_get_error(\c 0) returns a JSON object describing the last
 * error occurred in Kunquat Handle processing in general. In a
 * single-threaded application, you can always call
 * kqt_Handle_get_error(\c 0) to get the last error message, whether or
 * not related to any particular Handle.
 *
 * \param handle   The Handle, or \c 0 if retrieving error information
 *                 that is not necessarily associated with a Kunquat Handle.
 *
 * \return   The last error description. This is an empty string if no error
 *           has occurred.
 */
const char* kqt_Handle_get_error(kqt_Handle handle);


/**
 * Get human-readable error message from the Kunquat Handle.
 *
 * \param handle   The Handle, or \c 0 if retrieving error information that is
 *                 not necessarily associated with a Kunquat Handle.
 *
 * \return   The latest error message. This is an empty string if no error has
 *           occurred. The same message is also contained in the JSON object
 *           returned by \a kqt_Handle_get_error.
 */
const char* kqt_Handle_get_error_message(kqt_Handle handle);


/**
 * Clear error information of the Kunquat Handle.
 *
 * Validation errors are not cleared from Handles as they are considered fatal
 * errors.
 *
 * \param handle   The Handle, or \c 0 for clearing the generic error
 *                 description.
 */
void kqt_Handle_clear_error(kqt_Handle handle);


/**
 * Validate the Kunquat Handle.
 *
 * This function needs to be called after one or more successful calls of
 * kqt_Handle_set_data before the Handle can be fully utilised again.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   \c 1 if successful, \c 0 if failed. If the validation fails,
 *           the Handle can no longer be used and should be deallocated by
 *           calling kqt_del_Handle(\a handle).
 */
int kqt_Handle_validate(kqt_Handle handle);


/**
 * Free all the resources allocated for an existing Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 */
void kqt_del_Handle(kqt_Handle handle);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_HANDLE_H


