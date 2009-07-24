

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


#define KQT_CONTEXT_ERROR_LENGTH (256)

#define POSITION_LENGTH (64)


struct kqt_Handle
{
    Song* song;
    Playdata* play;
    Playdata* play_silent;
    Voice_pool* voices;
    char error[KQT_CONTEXT_ERROR_LENGTH];
    char position[POSITION_LENGTH];
};


void kqt_Handle_set_error(kqt_Handle* handle, char* message, ...);

void kqt_Handle_stop(kqt_Handle* handle);


bool handle_is_valid(kqt_Handle* handle);

#define check_handle(handle, func, ret)                                  \
    do                                                                   \
    {                                                                    \
        if (!handle_is_valid((handle)))                                  \
        {                                                                \
            kqt_Handle_set_error(NULL,                                   \
                    func ": Invalid Kunquat Handle: %p", (void*)handle); \
            return (ret);                                                \
        }                                                                \
    } while (false)

#define check_handle_void(handle, func)                                  \
    do                                                                   \
    {                                                                    \
        if (!handle_is_valid((handle)))                                  \
        {                                                                \
            kqt_Handle_set_error(NULL,                                   \
                    func ": Invalid Kunquat Handle: %p", (void*)handle); \
            return;                                                      \
        }                                                                \
    } while (false)


#endif // KQT_HANDLE_PRIVATE_H


