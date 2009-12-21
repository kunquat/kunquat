

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


#ifndef KQT_HANDLE_PRIVATE_H
#define KQT_HANDLE_PRIVATE_H


#include <stdbool.h>

#include <kunquat/Handle.h>
#include <kunquat/Player_ext.h>

#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>
#include <File_tree.h>


#define KQT_CONTEXT_ERROR_LENGTH (256)

#define POSITION_LENGTH (64)


struct kqt_Handle
{
    Song* song;
    kqt_Access_mode mode;
    void* (*get_data)(kqt_Handle* handle, const char* key);
    long (*get_data_length)(kqt_Handle* handle, const char* key);
    void (*destroy)(struct kqt_Handle* handle);
    char error[KQT_CONTEXT_ERROR_LENGTH];
    char position[POSITION_LENGTH];
};


/**
 * Initialises a Kunquat Handle.
 *
 * \param handle        The Kunquat Handle -- must not be \c NULL.
 * \param buffer_size   The size of the mixing buffers -- must be positive.
 * \param tree          The File tree of the Kunquat composition -- must not
 *                      be \c NULL.
 *
 * \return   \c true if successful. Otherwise, \c false is returned and Handle
 *           error is set to indicate the error.
 */
bool kqt_Handle_init(kqt_Handle* handle, long buffer_size, File_tree* tree);


/**
 * Sets an error message for a Kunquat Handle.
 *
 * \param handle    The Kunquat Handle, or \c NULL if not applicable.
 * \param message   The error message format -- must not be \c NULL. This and
 *                  the extra arguments correspond to the arguments of the
 *                  printf family of functions.
 */
void kqt_Handle_set_error(kqt_Handle* handle, const char* message, ...);


/**
 * Resets the playback pointer of the Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL.
 */
void kqt_Handle_stop(kqt_Handle* handle);


/**
 * Checks the validity of a Kunquat Handle.
 *
 * \param handle   The pointer of the supposed Kunquat Handle.
 *
 * \return   \c true if \a handle is a valid Kunquat Handle, otherwise
 *           \c false.
 */
bool handle_is_valid(kqt_Handle* handle);


#define check_handle(handle, ret)                                        \
    if (true)                                                            \
    {                                                                    \
        if (!handle_is_valid((handle)))                                  \
        {                                                                \
            kqt_Handle_set_error(NULL, "%s: Invalid Kunquat Handle: %p", \
                    __func__, (void*)handle);                            \
            return (ret);                                                \
        }                                                                \
    } else (void)0

#define check_handle_void(handle)                                        \
    if (true)                                                            \
    {                                                                    \
        if (!handle_is_valid((handle)))                                  \
        {                                                                \
            kqt_Handle_set_error(NULL, "%s: Invalid Kunquat Handle: %p", \
                    __func__, (void*)handle);                            \
            return;                                                      \
        }                                                                \
    } else (void)0


bool is_valid_key(const char* key);


#define check_key(handle, key)                                          \
    if (true)                                                           \
    {                                                                   \
        assert(handle != NULL);                                         \
        if (key == NULL)                                                \
        {                                                               \
            kqt_Handle_set_error(handle, "%s: key must not be NULL",    \
                    __func__);                                          \
            return false;                                               \
        }                                                               \
        if (!is_valid_key(key))                                         \
        {                                                               \
            kqt_Handle_set_error(handle, "%s: key \"%s\" is not valid", \
                    __func__, key);                                     \
            return false;                                               \
        }                                                               \
    }                                                                   \
    else (void)0


#endif // KQT_HANDLE_PRIVATE_H


