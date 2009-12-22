

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

#include <Handle_private.h>

#include <xmemory.h>


static void* Handle_r_get_data(kqt_Handle* handle, const char* key);

static long Handle_r_get_data_length(kqt_Handle* handle, const char* key);

static void del_Handle_r(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle_r(long buffer_size, char* path)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, "%s: buffer_size must be positive",
                __func__);
        return NULL;
    }
    if (path == NULL)
    {
        kqt_Handle_set_error(NULL, "%s: path must not be NULL", __func__);
        return NULL;
    }
    kqt_Handle* handle = xalloc(kqt_Handle);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't allocate memory for new"
                " Kunquat Handle", __func__);
        return NULL;
    }
    File_tree* tree = new_File_tree_from_tar(path, NULL);
    if (tree == NULL)
    {
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


static void* Handle_r_get_data(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    return NULL; // TODO: implement
}


static long Handle_r_get_data_length(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    return -1; // TODO: implement
}


static void del_Handle_r(kqt_Handle* handle)
{
    assert(handle != NULL);
    assert(handle->mode == KQT_READ);
    xfree(handle);
    return;
}


