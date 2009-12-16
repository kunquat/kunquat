

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


static char* Handle_r_get_data(kqt_Handle* handle, char* key);

static long Handle_r_get_data_length(kqt_Handle* handle, char* key);

static void del_Handle_r(kqt_Handle* handle);


bool kqt_Handle_init(kqt_Handle* handle, long buffer_size, File_tree* tree)
{
    assert(handle != NULL);
    assert(buffer_size > 0);
    assert(tree != NULL);
    if (!add_handle(handle))
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for a new Kunquat Handle");
        return false;
    }
    handle->mode = KQT_READ;
    handle->song = NULL;
    handle->destroy = NULL;
    handle->error[0] = handle->error[KQT_CONTEXT_ERROR_LENGTH - 1] = '\0';
    handle->position[0] = handle->position[POSITION_LENGTH - 1] = '\0';

    int buffer_count = 2;
//    int voice_count = 256;
    int event_queue_size = 32;

    handle->song = new_Song(buffer_count, buffer_size, event_queue_size);
    if (handle->song == NULL)
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for a new Kunquat Handle");
        return false;
    }
    kqt_Handle_stop(handle);
    kqt_Handle_set_position_desc(handle, NULL);

    if (!Song_read(handle->song, tree, state))
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't initialise the"
                             " Kunquat Handle: %s:%d: %s",
                             state->path, state->row, state->message);
        del_Song(handle->song);
        handle->song = NULL;
        return false;
    }
    return true;
}


kqt_Handle* kqt_new_Handle_r(long buffer_size, char* path)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, __func__ ": buffer_size must be positive");
        return NULL;
    }
    if (path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": path must not be NULL");
        return NULL;
    }
    kqt_Handle* handle = xalloc(kqt_Handle);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for new Kunquat Handle");
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    File_tree* tree = new_File_tree_from_tar(path, state);
    if (tree == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't load the path %s"
                " as a Kunquat composition file: %s:%d: %s", path,
                state->path, state->row, state->message);
        xfree(handle);
        return NULL;
    }
    if (!kqt_Handle_init(handle, buffer_size, tree))
    {
        del_File_tree(tree);
        xfree(handle);
        return NULL;
    }
    del_File_tree(tree);
    handle->mode = KQT_READ;
    handle->get_data = Handle_r_get_data;
    handle->get_data_length = Handle_r_get_data_length;
    handle->destroy = del_Handle_r;
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


char* kqt_Handle_get_data(kqt_Handle* handle, char* key)
{
    assert(handle->get_data != NULL);
    check_handle(handle, NULL);
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": key must not be NULL");
        return NULL;
    }
    return handle->get_data(handle, key);
}


long kqt_Handle_get_data_length(kqt_Handle* handle, char* key)
{
    assert(handle->get_data_length != NULL);
    check_handle(handle, NULL);
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": key must not be NULL");
        return NULL;
    }
    return handle->get_data_length(handle, key);
}


void kqt_del_Handle(kqt_Handle* handle)
{
    check_handle_void(handle);
    if (!remove_handle(handle))
    {
        kqt_Handle_set_error(NULL,
                __func__ ": Invalid Kunquat Handle: %p", (void*)handle);
        return;
    }
    if (handle->song != NULL)
    {
        del_Song(handle->song);
        handle->song = NULL;
    }
    assert(handle->destroy != NULL);
    handle->destroy(handle);
    return;
}


static char* Handle_r_get_data(kqt_Handle* handle, char* key)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    return NULL; // TODO: implement
}


static long Handle_r_get_data_length(kqt_Handle* handle, char* key)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    return -1; // TODO: implement
}


static void del_Handle_r(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ);
    (void)handle;
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


