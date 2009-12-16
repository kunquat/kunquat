

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
    char* get_data(kqt_Handle* handle, char* key);
    char* get_data_length(kqt_Handle* handle, char* key);
    void destroy(struct kqt_Handle* handle);
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


void kqt_Handle_set_error(kqt_Handle* handle, char* message, ...);

void kqt_Handle_stop(kqt_Handle* handle);


bool handle_is_valid(kqt_Handle* handle);

#define check_handle(handle, ret)                                   \
    if (true)                                                       \
    {                                                               \
        if (!handle_is_valid((handle)))                             \
        {                                                           \
            kqt_Handle_set_error(NULL, __func__                     \
                    ": Invalid Kunquat Handle: %p", (void*)handle); \
            return (ret);                                           \
        }                                                           \
    } else (void)0

#define check_handle_void(handle)                                   \
    if (true)                                                       \
    {                                                               \
        if (!handle_is_valid((handle)))                             \
        {                                                           \
            kqt_Handle_set_error(NULL, __func__                     \
                    ": Invalid Kunquat Handle: %p", (void*)handle); \
            return;                                                 \
        }                                                           \
    } else (void)0


#endif // KQT_HANDLE_PRIVATE_H


