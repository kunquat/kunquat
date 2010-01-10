

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
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
static char null_error[KQT_HANDLE_ERROR_LENGTH] = { '\0' };


static bool add_handle(kqt_Handle* handle);

static bool remove_handle(kqt_Handle* handle);


bool kqt_Handle_init(kqt_Handle* handle, long buffer_size)
{
    assert(handle != NULL);
    assert(buffer_size > 0);
    if (!add_handle(handle))
    {
        return false;
    }
    handle->mode = KQT_READ;
    handle->song = NULL;
    handle->destroy = NULL;
    handle->get_data = NULL;
    handle->get_data_length = NULL;
    handle->error[0] = handle->error[KQT_HANDLE_ERROR_LENGTH - 1] = '\0';
    handle->position[0] = handle->position[POSITION_LENGTH - 1] = '\0';

    int buffer_count = SONG_DEFAULT_BUF_COUNT;
//    int voice_count = 256;
    int event_queue_size = 32;

    handle->song = new_Song(buffer_count, buffer_size, event_queue_size);
    if (handle->song == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
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


void kqt_Handle_clear_error(kqt_Handle* handle)
{
    if (!handle_is_valid(handle))
    {
        null_error[0] = '\0';
        return;
    }
    handle->error[0] = '\0';
    return;
}


void kqt_Handle_set_error_(kqt_Handle* handle,
                           Error_type type,
                           const char* func,
                           const char* message, ...)
{
    assert(type > ERROR_NONE);
    assert(type < ERROR_LAST);
    assert(func != NULL);
    assert(message != NULL);
    char err_str[KQT_HANDLE_ERROR_LENGTH] = { '\0' };
    static const char* error_codes[ERROR_LAST] =
    {
        [ERROR_ARGUMENT] = "ArgumentError",
        [ERROR_FORMAT] = "FormatError",
        [ERROR_MEMORY] = "MemoryError",
        [ERROR_RESOURCE] = "ResourceError",
    };
    strcpy(err_str, error_codes[type]);
    strcat(err_str, ": ");
#ifndef NDEBUG
    strcat(err_str, "@");
    strcat(err_str, func);
    strcat(err_str, ": ");
#else
    (void)func;
#endif
    int error_code_length = strlen(err_str);
    va_list args;
    va_start(args, message);
    vsnprintf(err_str + error_code_length,
              KQT_HANDLE_ERROR_LENGTH - error_code_length,
              message, args);
    va_end(args);
    err_str[KQT_HANDLE_ERROR_LENGTH - 1] = '\0';

    strcpy(null_error, err_str);
    if (handle != NULL)
    {
        assert(handle_is_valid(handle));
        strcpy(handle->error, err_str);
    }
    return;
}


void* kqt_Handle_get_data(kqt_Handle* handle, const char* key)
{
    assert(handle->get_data != NULL);
    check_handle(handle, NULL);
    check_key(handle, key, NULL);
    return handle->get_data(handle, key);
}


long kqt_Handle_get_data_length(kqt_Handle* handle, const char* key)
{
    assert(handle->get_data_length != NULL);
    check_handle(handle, -1);
    check_key(handle, key, -1);
    return handle->get_data_length(handle, key);
}


bool key_is_valid(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "No key given");
        return false;
    }
    if (strlen(key) > KQT_KEY_LENGTH_MAX)
    {
        char key_repr[KQT_KEY_LENGTH_MAX + 3] = { '\0' };
        strncpy(key_repr, key, KQT_KEY_LENGTH_MAX - 1);
        strcat(key_repr, "...");
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s is too long"
                " (over %d characters)", key_repr, KQT_KEY_LENGTH_MAX);
        return false;
    }
    bool valid_element = false;
    bool element_has_period = false;
    const char* key_iter = key;
    while (*key_iter != '\0')
    {
        if (!(*key_iter >= '0' && *key_iter <= '9') &&
                strchr("abcdefghijklmnopqrstuvwxyz_./X", *key_iter) == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains an"
                    " illegal character \'%c\'", key, *key_iter);
            return false;
        }
        if (*key_iter != '.' && *key_iter != '/')
        {
            valid_element = true;
        }
        else if (*key_iter == '.')
        {
            element_has_period = true;
        }
        else if (*key_iter == '/')
        {
            if (!valid_element)
            {
                kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains"
                        " an invalid component", key);
                return false;
            }
            else if (element_has_period)
            {
                kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains"
                        " an intermediate component with a period", key);
                return false;
            }
            valid_element = false;
            element_has_period = false;
        }
        ++key_iter;
    }
    if (!element_has_period)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "The final element of"
                " key %s does not have a period", key);
        return false;
    }
    return true;
}


void kqt_del_Handle(kqt_Handle* handle)
{
    check_handle_void(handle);
    if (!remove_handle(handle))
    {
        kqt_Handle_set_error(NULL, ERROR_ARGUMENT,
                "Invalid Kunquat Handle: %p", (void*)handle);
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
    kqt_Handle_set_error(NULL, ERROR_MEMORY,
            "Maximum number of Kunquat Handles reached");
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


