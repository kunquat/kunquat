

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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <kunquat/limits.h>

#include <Directory.h>
#include <Handle_rw.h>

#include <xmemory.h>


static void del_Handle_rw(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle_rw(long buffer_size, char* path)
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
    Handle_rw* handle_rw = xalloc(Handle_rw);
    if (handle_rw == NULL)
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't allocate memory for a new"
                " Kunquat Handle", __func__);
        return NULL;
    }
    handle_rw->base_path = xnalloc(char, strlen(path) + 1);
    if (handle_rw->base_path == NULL)
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't allocate memory for a new"
                " Kunquat Handle", __func__);
        xfree(handle_rw);
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    File_tree* tree = new_File_tree_from_fs(path, state);
    if (tree == NULL)
    {
        kqt_Handle_set_error(NULL, "%s: Couldn't load the path %s as a"
                " Kunquat composition directory: %s:%d: %s", __func__, path,
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
    return &handle_rw->handle;
}


int kqt_Handle_rw_set_data(kqt_Handle* handle,
                           char* key,
                           void* data,
                           int length)
{
    check_handle(handle, 0);
    if (handle->mode == KQT_READ)
    {
        kqt_Handle_set_error(handle, "%s: Cannot set data on a read-only"
                " Kunquat Handle.", __func__);
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


static bool path_element_is_header(const char* element)
{
    assert(element != NULL);
    return strncmp("kunquat", element, 7) == 0 &&
           (element[7] == 'i' || element[7] == 's') &&
           strcmp("XX", element + 8) == 0 &&
           element[10] == '/';
}


static char* Handle_rw_get_real_path(Handle_rw* handle_rw, const char* key_path)
{
    assert(handle_rw->handle.mode != KQT_READ);
    assert(key_path != NULL);
    char* real_path = xcalloc(char, strlen(key_path) + 1);
    if (real_path == NULL)
    {
        kqt_Handle_set_error(&handle_rw->handle, "%s: Couldn't allocate"
                " memory", __func__);
        return NULL;
    }
    strcpy(real_path, handle_rw->base_path);
    Path_type info = path_info(real_path, &handle_rw->handle);
    if (info == PATH_ERROR)
    {
        xfree(real_path);
        return NULL;
    }
    if (info != PATH_IS_DIR)
    {
        kqt_Handle_set_error(&handle_rw->handle, "%s: Base path %s of the Kunquat"
                " Handle is not a directory", __func__, handle_rw->base_path);
        xfree(real_path);
        return NULL;
    }
    char* cur_path = add_path_element(real_path, key_path);
    while (cur_path != NULL)
    {
        if (path_element_is_header(cur_path))
        {
            char* dir_path = xcalloc(char, strlen(cur_path) + 1);
            if (dir_path == NULL)
            {
                kqt_Handle_set_error(&handle_rw->handle, "%s: Couldn't"
                        " allocate memory", __func__);
                xfree(real_path);
                return NULL;
            }
            bool non_solidus_found = false;
            for (int i = strlen(dir_path) - 1; i >= 0; --i)
            {
                if (!non_solidus_found && dir_path[i] != '/')
                {
                    non_solidus_found = true;
                }
                else if (non_solidus_found && dir_path[i] == '/')
                {
                    break;
                }
                dir_path[i] = '\0';
            }
            Directory* dir = new_Directory(dir_path, &handle_rw->handle);
            int last_pos = strlen(dir_path);
            xfree(dir_path);
            if (dir == NULL)
            {
                xfree(real_path);
                return NULL;
            }
            char* entry = Directory_get_entry(dir);
            int max_version = -1;
            while (entry != NULL)
            {
                if (strncmp("kunquat", entry + last_pos, 7) == 0 &&
                        ((entry + last_pos)[7] == 'i' || (entry + last_pos)[7] == 's') &&
                        isdigit((entry + last_pos)[8]) && isdigit((entry + last_pos)[9]))
                {
                    int version = 10 * ((entry + last_pos)[8] - '0');
                    version += ((entry + last_pos)[9] - '0');
                    if (version > max_version)
                    {
                        max_version = version;
                    }
                }
                xfree(entry);
                entry = Directory_get_entry(dir);
            }
            del_Directory(dir);
            if (kqt_Handle_get_error(&handle_rw->handle)[0] != '\0')
            {
                xfree(real_path);
                return NULL;
            }
            if (max_version == -1)
            {
                return real_path;
            }
            snprintf(cur_path + 8, 2, "%02d", max_version);
        }
        cur_path = add_path_element(real_path, key_path);
    }
    return real_path;
}


void* Handle_rw_get_data(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(handle->mode != KQT_READ);
    assert(key != NULL);
    Handle_rw* handle_rw = (Handle_rw*)handle;
    char* key_path = append_to_path(handle_rw->base_path, key);
    if (key_path == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        return NULL;
    }
    char* real_path = Handle_rw_get_real_path(handle_rw, key_path);
    xfree(key_path);
    if (real_path == NULL)
    {
        return NULL;
    }
    Path_type info = path_info(real_path, handle);
    if (info == PATH_NOT_EXIST)
    {
        xfree(real_path);
        return NULL;
    }
    if (info != PATH_IS_REGULAR)
    {
        kqt_Handle_set_error(handle, "%s: Key %s does not correspond"
                " to a regular file", __func__, key);
        xfree(real_path);
        return NULL;
    }
    long size = path_size(real_path, handle);
    if (size <= 0)
    {
        xfree(real_path);
        return NULL;
    }
    char* data = xnalloc(char, size);
    if (data == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        xfree(real_path);
        return NULL;
    }
    errno = 0;
    FILE* in = fopen(real_path, "rb");
    xfree(real_path);
    if (in == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't read the value of"
                " key %s: %s", __func__, key, strerror(errno));
        xfree(data);
        return NULL;
    }
    for (long i = 0; i < size; ++i)
    {
        int ch = fgetc(in);
        if (ch == EOF)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't read all the"
                    " data in %s", __func__, key);
            xfree(data);
            return NULL;
        }
        data[i] = ch;
    }
    fclose(in);
    return data;
}


long Handle_rw_get_data_length(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    assert(handle->mode != KQT_READ);
    assert(key != NULL);
    Handle_rw* handle_rw = (Handle_rw*)handle;
    char* key_path = append_to_path(handle_rw->base_path, key);
    if (key_path == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        return -1;
    }
    char* real_path = Handle_rw_get_real_path(handle_rw, key_path);
    xfree(key_path);
    if (real_path == NULL)
    {
        return -1;
    }
    long size = path_size(real_path, handle);
    xfree(real_path);
    return size;
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
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        return 0;
    }
    char* real_path = xcalloc(char, strlen(key_path) + 1);
    if (real_path == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
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
        kqt_Handle_set_error(handle, "%s: Base path %s of the Kunquat"
                " Handle is not a directory", __func__, handle_rw->base_path);
        xfree(key_path);
        xfree(real_path);
        return 0;
    }
    char* cur_path = add_path_element(real_path, key_path);
    while (cur_path != NULL)
    {
        if (path_element_is_header(cur_path))
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
            kqt_Handle_set_error(handle, "%s: Path component %s of the"
                    " key %s is not a directory", __func__, real_path, key);
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
        kqt_Handle_set_error(handle, "%s: Key %s exists but is not a"
                " regular file", __func__, key);
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
                kqt_Handle_set_error(handle, "%s: Couldn't set the key %s:"
                        " %s", __func__, key, strerror(errno));
                xfree(real_path);
                return 0;
            }
        }
        return 1;
    }
    FILE* out = fopen(real_path, "wb");
    xfree(real_path);
    if (out == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't write the key %s: %s",
                __func__, key, strerror(errno));
        return 0;
    }
    char* bytes = (char*)data;
    for (long i = 0; i < length; ++i)
    {
        if (fputc(*bytes, out) == EOF)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't write the key %s",
                    __func__, key);
            return 0;
        }
        ++bytes;
    }
    if (fclose(out) == EOF)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't finalise writing of the"
                " key %s -- data may be lost", __func__, key);
        return 0;
    }
    return 1;
}


static void del_Handle_rw(kqt_Handle* handle)
{
    assert(handle != NULL);
    assert(handle->mode == KQT_READ_WRITE);
    Handle_rw* handle_rw = (Handle_rw*)handle;
    xfree(handle_rw->base_path);
    xfree(handle_rw);
    return;
}


