

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


#endif // KQT_HANDLE_RW_H


