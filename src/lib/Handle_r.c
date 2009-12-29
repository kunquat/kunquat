

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
#include <string.h>

#include <Handle_private.h>
#include <Handle_r.h>
#include <File_tree.h>

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
    Handle_r* handle_r = xalloc(Handle_r);
    if (handle_r == NULL)
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't allocate memory for a new"
                " Kunquat Handle", __func__);
        return NULL;
    }
    handle_r->handle.mode = KQT_READ;
    handle_r->tree = new_File_tree_from_tar(path, NULL);
    if (handle_r->tree == NULL)
    {
        del_Handle_r(&handle_r->handle);
        return NULL;
    }
    if (!kqt_Handle_init(&handle_r->handle, buffer_size, handle_r->tree))
    {
        del_Handle_r(&handle_r->handle);
        return NULL;
    }
    handle_r->handle.get_data = Handle_r_get_data;
    handle_r->handle.get_data_length = Handle_r_get_data_length;
    handle_r->handle.destroy = del_Handle_r;
    return &handle_r->handle;
}


static void* Handle_r_get_data(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ);
    assert(key_is_valid(key));
    Handle_r* handle_r = (Handle_r*)handle;
    bool error = false;
    File_tree* tree = File_tree_get_node(handle_r->tree, key, &error);
    if (tree == NULL)
    {
        if (error)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
        }
        return NULL;
    }
    if (File_tree_is_dir(tree))
    {
        kqt_Handle_set_error(handle, "%s: Key %s does not correspond"
                " to a regular file", __func__, key);
        return NULL;
    }
    if (!File_tree_is_regular(tree))
    {
        kqt_Handle_set_error(handle, "%s: Key %s is a sample -- reading raw"
                " sample data from a read-only Handle is not supported yet",
                __func__, key);
        return NULL;
    }
    char* data = File_tree_get_data(tree);
    long size = File_tree_get_size(tree);
    if (size <= 0)
    {
        return NULL;
    }
    assert(data != NULL);
    char* new_data = xcalloc(char, size);
    if (new_data == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        return NULL;
    }
    if (data != NULL)
    {
        memcpy(new_data, data, size);
    }
    return new_data;
}


static long Handle_r_get_data_length(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ);
    assert(key_is_valid(key));
    Handle_r* handle_r = (Handle_r*)handle;
    bool error = false;
    File_tree* tree = File_tree_get_node(handle_r->tree, key, &error);
    if (tree == NULL)
    {
        if (error)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
        }
        return -1;
    }
    if (File_tree_is_dir(tree))
    {
        kqt_Handle_set_error(handle, "%s: Key %s does not correspond"
                " to a regular file", __func__, key);
        return -1;
    }
    if (!File_tree_is_regular(tree))
    {
        kqt_Handle_set_error(handle, "%s: Key %s is a sample -- reading raw"
                " sample data from a read-only Handle is not supported yet",
                __func__, key);
        return -1;
    }
    assert(File_tree_get_size(tree) >= 0);
    return File_tree_get_size(tree);
}


static void del_Handle_r(kqt_Handle* handle)
{
    assert(handle != NULL);
    assert(handle->mode == KQT_READ);
    Handle_r* handle_r = (Handle_r*)handle;
    if (handle_r->tree != NULL)
    {
        del_File_tree(handle_r->tree);
    }
    xfree(handle_r);
    return;
}


