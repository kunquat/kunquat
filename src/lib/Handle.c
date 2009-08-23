

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


#define _POSIX_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <Handle_private.h>

#include <kunquat/limits.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>

#include <xmemory.h>


static kqt_Handle* handles[KQT_HANDLES_MAX] = { NULL };


// For errors without an associated Kunquat Handle.
static char null_error[KQT_CONTEXT_ERROR_LENGTH] = { '\0' };


static bool add_handle(kqt_Handle* handle);

static bool remove_handle(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle(long buffer_size)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, "kqt_new_Handle: buf_size must be positive");
        return NULL;
    }
    kqt_Handle* handle = xalloc(kqt_Handle);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }
    if (!add_handle(handle))
    {
        kqt_Handle_set_error(NULL, "Maximum amount of simultaneous Kunquat Handles reached");
        xfree(handle);
        return NULL;
    }
    handle->song = NULL;
    handle->error[0] = handle->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    handle->position[0] = handle->position[POSITION_LENGTH - 1] = '\0';

    int buffer_count = 2;
//    int voice_count = 256;
    int event_queue_size = 32;

    handle->song = new_Song(buffer_count, buffer_size, event_queue_size);
    if (handle->song == NULL)
    {
        kqt_del_Handle(handle);
        kqt_Handle_set_error(NULL, "Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }

    kqt_Handle_stop(handle);
    kqt_Handle_set_position_desc(handle, NULL);
    return handle;
}


kqt_Handle* kqt_new_Handle_from_path(long buffer_size, char* path)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, "kqt_new_Handle_from_path: buf_size must be positive");
        return NULL;
    }
    if (path == NULL)
    {
        kqt_Handle_set_error(NULL, "kqt_new_Handle_from_path: path must not be NULL");
        return NULL;
    }
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        kqt_Handle_set_error(NULL, "Couldn't access %s: %s", path, strerror(errno));
        return NULL;
    }
    kqt_Handle* handle = kqt_new_Handle(buffer_size);
    if (handle == NULL)
    {
        return NULL;
    }
    File_tree* tree = NULL;
    Read_state* state = READ_STATE_AUTO;
    if (S_ISDIR(info->st_mode))
    {
        tree = new_File_tree_from_fs(path, state);
        if (tree == NULL)
        {
            kqt_Handle_set_error(NULL, "%s:%d: %s",
                                  state->path, state->row, state->message);
            kqt_del_Handle(handle);
            return NULL;
        }
    }
    else
    {
        tree = new_File_tree_from_tar(path, state);
        if (tree == NULL)
        {
            kqt_Handle_set_error(NULL, "%s:%d: %s",
                                  state->path, state->row, state->message);
            kqt_del_Handle(handle);
            return NULL;
        }
    }
    assert(tree != NULL);
    if (!Song_read(handle->song, tree, state))
    {
        kqt_Handle_set_error(NULL, "%s:%d: %s",
                              state->path, state->row, state->message);
        del_File_tree(tree);
        kqt_del_Handle(handle);
        return NULL;
    }
    del_File_tree(tree);
    kqt_Handle_stop(handle);
    kqt_Handle_set_position_desc(handle, NULL);
    return handle;
}


char* kqt_Handle_get_error(kqt_Handle* handle)
{
    if (!handle_is_valid(handle))
    {
        return null_error;
    }
    return handle->error;
}


void kqt_Handle_set_error(kqt_Handle* handle, char* message, ...)
{
    assert(message != NULL);
    va_list args;
    va_start(args, message);
    vsnprintf(null_error, KQT_CONTEXT_ERROR_LENGTH, message, args);
    va_end(args);
    null_error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    if (handle != NULL)
    {
        assert(handle_is_valid(handle));
        va_start(args, message);
        vsnprintf(handle->error, KQT_CONTEXT_ERROR_LENGTH, message, args);
        va_end(args);
        handle->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    }
    return;
}


void kqt_del_Handle(kqt_Handle* handle)
{
    if (!handle_is_valid(handle))
    {
        kqt_Handle_set_error(NULL,
                "kqt_del_Handle: Invalid Kunquat Handle: %p", (void*)handle);
        return;
    }
    if (!remove_handle(handle))
    {
        kqt_Handle_set_error(NULL,
                "kqt_del_Handle: Invalid Kunquat Handle: %p", (void*)handle);
        return;
    }
    if (handle->song != NULL)
    {
        del_Song(handle->song);
    }
    xfree(handle);
    return;
}


static bool add_handle(kqt_Handle* handle)
{
    assert(handle != NULL);
#ifndef NDEBUG
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        assert(handles[i] != handle);
    }
#endif
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        if (handles[i] == NULL)
        {
            handles[i] = handle;
            return true;
        }
    }
    return false;
}


bool handle_is_valid(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        return false;
    }
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        if (handles[i] == handle)
        {
#ifndef NDEBUG
            for (int k = i + 1; k < KQT_HANDLES_MAX; ++k)
            {
                assert(handles[k] != handle);
            }
#endif
            return true;
        }
    }
    return false;
}


static bool remove_handle(kqt_Handle* handle)
{
    assert(handle != NULL);
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        if (handles[i] == handle)
        {
            handles[i] = NULL;
#ifndef NDEBUG
            for (int k = i + 1; k < KQT_HANDLES_MAX; ++k)
            {
                assert(handles[k] != handle);
            }
#endif
            return true;
        }
    }
    return false;
}


