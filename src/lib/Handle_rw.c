

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
#include <stdio.h>
#include <errno.h>

#include <kunquat/limits.h>

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
    Handle_rw* handle_rw = xalloc(Handle_rw);
    if (handle_rw == NULL)
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for new Kunquat Handle");
        return NULL;
    }
    handle_rw->base_path = xnalloc(char, strlen(path) + 1);
    if (handle_rw->base_path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__
                ": Couldn't allocate memory for new Kunquat Handle");
        xfree(handle_rw);
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    File_tree* tree = new_File_tree_from_fs(path, state);
    if (tree == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't load the path %s"
                " as a Kunquat composition directory: %s:%d: %s", path,
                state->path, state->row, state->message);
        xfree(handle_rw->base_path);
        xfree(handle_rw);
        return NULL;
    }
    if (!kqt_Handle_init(&handle_rw->handle, buffer_size, tree))
    {
        del_File_tree(tree);
        xfree(handle_rw->base_path);
        xfree(handle_rw);
        return NULL;
    }
    del_File_tree(tree);
    handle_rw->handle.mode = KQT_READ_WRITE;
    handle_rw->handle.get_data = Handle_rw_get_data;
    handle_rw->handle.get_data_length = Handle_rw_get_data_length;
    handle_rw->set_data = Handle_rw_set_data;
    handle_rw->handle.destroy = del_Handle_rw;
    return &handle->handle;
}


int kqt_Handle_rw_set_data(kqt_Handle* handle,
                           char* key,
                           void* data,
                           int length)
{
    check_handle(handle, 0);
    if (handle->mode == KQT_READ)
    {
        kqt_Handle_set_error(handle, __func__
                ": Cannot set data on a read-only Kunquat Handle.");
        return 0;
    }
    Handle_rw* handle_rw = (Handle_rw*)handle;
    assert(handle_rw->set_data != NULL);
    return handle_rw->set_data(handle, key, data, length);
}


char* add_path_element(char* partial_path, const char* full_path)
{
    assert(partial_path != NULL);
    assert(full_path != NULL);
    int partial_len = strlen(partial_path);
    char* ret = NULL;
    int full_len = strlen(full_path);
    int i = partial_len;
    for (; i < full_len; ++i)
    {
        partial_path[i] = full_path[i];
        if (ret == NULL && full_path[i] != '/')
        {
            ret = &partial_path[i];
        }
        else if (ret != NULL && full_path[i] == '/')
        {
            partial_path[i + 1] = '\0';
            return ret;
        }
    }
    partial_path[i] = '\0';
    return ret;
}


static void* Handle_rw_get_data(kqt_Handle* handle, const char* key)
{
}


static long Handle_rw_get_data_length(kqt_Handle* handle, const char* key)
{
}


int Handle_rw_set_data(kqt_Handle* handle,
                       const char* key,
                       void* data,
                       long length)
{
    assert(handle_is_valid(handle));
    assert(handle->mode != KQT_READ);
    assert(key != NULL);
    assert(data != NULL || length == 0);
    assert(length >= 0);
    bool remove_key = length == 0;
    Handle_rw* handle_rw = (Handle_rw*)handle;
    char* key_path = append_to_path(handle_rw->base_path, key);
    if (key_path == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't allocate memory");
        return 0;
    }
    char* real_path = xcalloc(char, strlen(key_path) + 1);
    if (real_path == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't allocate memory");
        xfree(key_path);
        return 0;
    }
    strcpy(real_path, handle_rw->base_path);
    Path_type info = path_info(real_path, handle);
    if (info == PATH_ERROR)
    {
        xfree(key_path);
        xfree(real_path);
        return 0;
    }
    if (info != PATH_IS_DIR)
    {
        kqt_Handle_set_error(handle, __func__ ": Base path %s of the Kunquat"
                " Handle is not a directory", handle_rw->base_path);
        xfree(key_path);
        xfree(real_path);
        return 0;
    }
    char* cur_path = add_path_element(real_path, key_path);
    while (cur_path != NULL)
    {
        if (strncmp("kunquat", cur_path, 7) == 0 &&
                (cur_path[7] == 'i' || cur_path[7] == 's') &&
                strcmp("XX", cur_path + 8) == 0 &&
                cur_path[10] == '/')
        {
            strcpy(cur_path + 8, KQT_FORMAT_VERSION);
        }
        bool cur_is_dir = cur_path[0] != '\0' &&
                          cur_path[strlen(cur_path) - 1] == '/';
        info = path_info(real_path, handle);
        if (info == PATH_ERROR)
        {
            xfree(key_path);
            xfree(real_path);
            return 0;
        }
        if (!cur_is_dir)
        {
            break;
        }
        if (info != PATH_NOT_EXIST && info != PATH_IS_DIR)
        {
            kqt_Handle_set_error(handle, __func__ ": Path component %s of the"
                    " key %s is not a directory", real_path, key);
            xfree(key_path);
            xfree(real_path);
            return 0;
        }
        if (info == PATH_NOT_EXIST)
        {
            if (remove_key)
            {
                xfree(key_path);
                xfree(real_path);
                return 1;
            }
            if (!create_dir(real_path, handle))
            {
                xfree(key_path);
                xfree(real_path);
                return 0;
            }
        }
        cur_path = add_path_element(real_path, key_path);
    }
    xfree(key_path);
    if (info != PATH_NOT_EXIST && info != PATH_IS_REGULAR)
    {
        kqt_Handle_set_error(handle, __func__ ": Key %s exists"
                " but is not a regular file", key);
        xfree(real_path);
        return 0;
    }
    errno = 0;
    if (remove_key)
    {
        if (info != PATH_NOT_EXIST)
        {
            if (remove(real_path) != 0)
            {
                kqt_Handle_set_error(handle, __func__ ": Couldn't set the"
                        " key %s: %s", key, strerror(errno));
                xfree(real_path);
                return 0;
            }
        }
        return 1;
    }
    FILE* out = fopen(real_path, "w");
    xfree(real_path);
    if (out == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't write"
                " the key %s: %s", key, strerror(errno));
        return 0;
    }
    char* bytes = (char*)data;
    for (long i = 0; i < length; ++i)
    {
        if (fputc(bytes, out) == EOF)
        {
            kqt_Handle_set_error(handle, __func__ ": Couldn't write"
                    " the key %s", key);
            return 0;
        }
        ++bytes;
    }
    if (fclose(out) == EOF)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't finalise"
                " writing of the key %s -- data may be lost", key);
        return 0;
    }
    return 1;
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


