

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef KQT_HANDLE_RW_H
#define KQT_HANDLE_RW_H


#include <Handle_private.h>


typedef struct Handle_rw
{
    kqt_Handle handle;
    char* base_path;
    int (*set_data)(kqt_Handle* handle,
                    const char* key,
                    void* data,
                    long length);
} Handle_rw;


/**
 * Gets data from the read/write Kunquat Handle associated with the given key.
 *
 * \param handle   The Kunquat Handle -- should be valid.
 * \param key      The key of the data -- should not be \c NULL.
 *
 * \return   The data if existent and no error occurred, otherwise \c NULL.
 *           Check kqt_Handle_error(handle) for errors.
 */
void* Handle_rw_get_data(kqt_Handle* handle, const char* key);


/**
 * Gets length of data from the read/write Kunquat Handle associated with the
 * given key.
 *
 * \param handle   The Kunquat Handle -- should be valid.
 * \param key      The key of the data -- should not be \c NULL.
 *
 * \return   The length of the data if successful. Otherwise, \c -1 is
 *           returned and Kunquat Handle error is set accordingly.
 */
long Handle_rw_get_data_length(kqt_Handle* handle, const char* key);


/**
 * Sets data into the read/write Handle associated with the given key.
 *
 * This function overwrites any data that is already associated with the key.
 *
 * \param handle   The read/write Kunquat Handle -- should not be \c NULL.
 * \param key      The key of the data -- should not be \c NULL.
 * \param data     The data to be set -- should not be \c NULL.
 * \param length   The length of the data in bytes -- should be >= \c 0.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int Handle_rw_set_data(kqt_Handle* handle,
                       const char* key,
                       void* data,
                       long length);


/**
 * Converts a virtual path into a real one.
 *
 * This function searches the given path for "kunquatiXX" or "kunquatsXX"
 * and replaces the "XX" with an existent version number if found.
 *
 * \param handle_rw   The read/write Kunquat Handle -- must not be \c NULL.
 * \param path        The virtual path -- must not be \c NULL. The path may
 *                    be modified in-place.
 *
 * \return   \c true if successful, otherwise \c false.
 */
// bool Handle_rw_resolve_path(Handle_rw* handle_rw, char* path);


#endif // KQT_HANDLE_RW_H


