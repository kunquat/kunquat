

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


bool kqt_Handle_init(kqt_Handle* handle, long buffer_size, File_tree* tree)
{
    assert(handle != NULL);
    assert(buffer_size > 0);
    assert(tree != NULL);
    if (!add_handle(handle))
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't allocate memory for a new"
                " Kunquat Handle", __func__);
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
        kqt_Handle_set_error(NULL, "%s: Couldn't allocate memory for a new"
                " Kunquat Handle", __func__);
        return false;
    }

    Read_state* state = READ_STATE_AUTO;
    if (!Song_read(handle->song, tree, state))
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't initialise the"
                             " Kunquat Handle: %s:%d: %s", __func__,
                             state->path, state->row, state->message);
        del_Song(handle->song);
        handle->song = NULL;
        return false;
    }
    kqt_Handle_stop(handle);
    kqt_Handle_set_position_desc(handle, NULL);
    return true;
}


char* kqt_Handle_get_error(kqt_Handle* handle)
{
    if (!handle_is_valid(handle))
    {
        return null_error;
    }
    return handle->error;
}


void kqt_Handle_set_error(kqt_Handle* handle, const char* message, ...)
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


void* kqt_Handle_get_data(kqt_Handle* handle, const char* key)
{
    assert(handle->get_data != NULL);
    check_handle(handle, NULL);
    check_key(handle, key);
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, "%s: key must not be NULL", __func__);
        return NULL;
    }
    return handle->get_data(handle, key);
}


long kqt_Handle_get_data_length(kqt_Handle* handle, const char* key)
{
    assert(handle->get_data_length != NULL);
    check_handle(handle, -1);
    check_key(handle, key);
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, "%s: key must not be NULL", __func__);
        return -1;
    }
    return handle->get_data_length(handle, key);
}


bool is_valid_key(const char* key)
{
    assert(key != NULL);
    bool valid_element = false;
    while (*key != '\0')
    {
        if (!(*key >= '0' && *key <= '9') &&
                strchr("abcdefghijklmnopqrstuvwxyz_./", *key) == NULL)
        {
            return false;
        }
        if (*key != '.' && *key != '/')
        {
            valid_element = true;
        }
        else if (*key == '/')
        {
            if (!valid_element)
            {
                return false;
            }
            valid_element = false;
        }
        ++key;
    }
    return true;
}


void kqt_del_Handle(kqt_Handle* handle)
{
    check_handle_void(handle);
    if (!remove_handle(handle))
    {
        kqt_Handle_set_error(NULL, "%s: Invalid Kunquat Handle: %p", __func__,
                (void*)handle);
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


