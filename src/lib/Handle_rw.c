

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

#include <Directory.h>

#include <Handle_rw.h>

#include <xmemory.h>


kqt_Handle* kqt_new_Handle_rw(long buffer_size, char* path)
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
#if 0
    Path_type info = path_info(path, NULL);
    if (info == PATH_ERROR)
    {
        return NULL;
    }
    if (info != PATH_IS_DIR)
    {
        kqt_Handle_set_error(NULL, __func__ ": path %s is not a directory", path);
        return NULL;
    }
#endif
    Handle_rw* handle = xalloc(Handle_rw);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for new Kunquat Handle");
        return NULL;
    }
    handle->base_path = xnalloc(char, strlen(path) + 1);
    if (handle->base_path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for new Kunquat Handle");
        xfree(handle);
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    File_tree* tree = new_File_tree_from_fs(path, state);
    if (tree == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't load the path %s"
                " as a Kunquat composition directory: %s:%d: %s", path,
                state->path, state->row, state->message);
        xfree(handle->base_path);
        xfree(handle);
        return NULL;
    }
    if (!kqt_Handle_init(&handle->handle, buffer_size, tree))
    {
        del_File_tree(tree);
        xfree(handle->base_path);
        xfree(handle);
        return NULL;
    }
    del_File_tree(tree);
    handle->handle.mode = KQT_READ_WRITE;
    handle->handle.get_data = Handle_rw_get_data;
    handle->handle.get_data_length = Handle_rw_get_data_length;
    handle->set_data = Handle_rw_set_data;
    handle->handle.destroy = del_Handle_rw;
    return &handle->handle;
}


static char* Handle_rw_get_data(kqt_Handle* handle, char* key)
{
}


static char* Handle_rw_get_data_length(kqt_Handle* handle, char* key)
{
}


static int Handle_rw_set_data(kqt_Handle* handle,
                              char* key,
                              void* data,
                              int length)
{
}


static void del_Handle_rw(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ_WRITE);
    Handle_rw* handle_rw = (Handle_rw*)handle;
    xfree(handle_rw->base_path);
    xfree(handle_rw);
    return;
}


