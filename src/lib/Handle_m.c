

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Handle_m.h>
#include <Handle_private.h>
#include <Parse_manager.h>
#include <xassert.h>
#include <xmemory.h>


static int Handle_m_set_data(kqt_Handle* handle,
                             const char* key,
                             void* data,
                             long length);

static void del_Handle_m(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle(void)
{
    Handle_m* handle_m = xalloc(Handle_m);
    if (handle_m == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        return NULL;
    }
    if (!kqt_Handle_init(&handle_m->handle, DEFAULT_BUFFER_SIZE))
    {
        del_Handle_m(&handle_m->handle);
        return NULL;
    }
    handle_m->handle.mode = KQT_MEM;
    handle_m->handle.set_data = Handle_m_set_data;
    handle_m->handle.destroy = del_Handle_m;
    if (!Device_sync((Device*)handle_m->handle.song))
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        kqt_del_Handle(&handle_m->handle);
        return NULL;
    }
    return &handle_m->handle;
}


static int Handle_m_set_data(kqt_Handle* handle,
                             const char* key,
                             void* data,
                             long length)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_MEM);
    assert(key_is_valid(handle, key));
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (!parse_data(handle, key, data, length))
    {
        return 0;
    }
    handle->data_is_validated = false;
    return 1;
}


static void del_Handle_m(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        return;
    }
    assert(handle->mode == KQT_MEM);
    Handle_m* handle_m = (Handle_m*)handle;
    xfree(handle_m);
    return;
}


