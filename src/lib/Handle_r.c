

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
#include <string.h>

#include <Handle_private.h>
#include <Handle_r.h>
#include <File_kqt.h>
#include <Entries.h>

#include <xmemory.h>


static void* Handle_r_get_data(kqt_Handle* handle, const char* key);

static long Handle_r_get_data_length(kqt_Handle* handle, const char* key);

static void del_Handle_r(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle_r(long buffer_size, char* path)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, ERROR_ARGUMENT,
                "Buffer size must be positive");
        return NULL;
    }
    if (path == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_ARGUMENT, "No input path given");
        return NULL;
    }
    Handle_r* handle_r = xalloc(Handle_r);
    if (handle_r == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        return NULL;
    }
    handle_r->handle.mode = KQT_READ;
    handle_r->entries = new_Entries();
    if (handle_r->entries == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        del_Handle_r(&handle_r->handle);
        return NULL;
    }
    if (!kqt_Handle_init(&handle_r->handle, buffer_size))
    {
        del_Handle_r(&handle_r->handle);
        return NULL;
    }
    handle_r->handle.mode = KQT_READ;
    handle_r->handle.get_data = Handle_r_get_data;
    handle_r->handle.get_data_length = Handle_r_get_data_length;
    handle_r->handle.destroy = del_Handle_r;
    if (!File_kqt_open(handle_r, path))
    {
        kqt_del_Handle(&handle_r->handle);
        return NULL;
    }
    return &handle_r->handle;
}


static void* Handle_r_get_data(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ);
    assert(key_is_valid(handle, key));
    Handle_r* handle_r = (Handle_r*)handle;
    int32_t length = Entries_get_length(handle_r->entries, key);
    if (length == 0)
    {
        return NULL;
    }
    void* data = Entries_get_data(handle_r->entries, key);
    assert(data != NULL);
    char* new_data = xcalloc(char, length);
    if (new_data == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory");
        return NULL;
    }
    if (data != NULL)
    {
        memcpy(new_data, data, length);
    }
    return new_data;
}


static long Handle_r_get_data_length(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ);
    assert(key_is_valid(handle, key));
    Handle_r* handle_r = (Handle_r*)handle;
    return Entries_get_length(handle_r->entries, key);
}


static void del_Handle_r(kqt_Handle* handle)
{
    assert(handle != NULL);
    assert(handle->mode == KQT_READ);
    Handle_r* handle_r = (Handle_r*)handle;
    if (handle_r->entries != NULL)
    {
        del_Entries(handle_r->entries);
    }
    xfree(handle_r);
    return;
}


