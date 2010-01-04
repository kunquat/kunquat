

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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

#include <Directory.h>
#include <Handle_rw.h>
#include <Handle_private.h>
#include <Parse_manager.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <math_common.h>

#include <xmemory.h>


static bool File_traverse_dir(Handle_rw* handle_rw, Directory* dir, int key_start);


bool File_dir_open(Handle_rw* handle_rw, const char* path)
{
    assert(handle_is_valid((kqt_Handle*)handle_rw));
    assert(path != NULL);
    char* abs_path = absolute_path(path, &handle_rw->handle);
    if (abs_path == NULL)
    {
        return false;
    }
    const char* header = "kunquatc" KQT_FORMAT_VERSION;
    const char* header_with_solidus = "kunquatc" KQT_FORMAT_VERSION "/";
    if (!string_has_suffix(abs_path, header) &&
            !string_has_suffix(abs_path, header_with_solidus))
    {
        kqt_Handle_set_error(&handle_rw->handle, "%s: Base path %s does not contain the"
                " header \"kunquatc" KQT_FORMAT_VERSION "\" as a final component",
                __func__, abs_path);
        xfree(abs_path);
        return false;
    }
    int key_start = strlen(abs_path);
    if (abs_path[key_start - 1] != '/')
    {
        ++key_start;
    }
    Directory* dir = new_Directory(abs_path, &handle_rw->handle);
    xfree(abs_path);
    if (dir == NULL)
    {
        return false;
    }
    if (!File_traverse_dir(handle_rw, dir, key_start))
    {
        del_Directory(dir);
        return false;
    }
    del_Directory(dir);
    return true;
}


static bool File_traverse_dir(Handle_rw* handle_rw, Directory* dir, int key_start)
{
    assert(handle_is_valid((kqt_Handle*)handle_rw));
    assert(dir != NULL);
    assert(key_start >= 0);
    char* entry = Directory_get_entry(dir);
    while (entry != NULL)
    {
        Path_type info = path_info(entry, &handle_rw->handle);
        if (info == PATH_ERROR)
        {
            xfree(entry);
            return false;
        }
        else if (info == PATH_IS_OTHER)
        {
            kqt_Handle_set_error(&handle_rw->handle, "%s: Path %s is"
                    " a foreign type of file", __func__, entry);
            xfree(entry);
            return false;
        }
        else if (info == PATH_IS_DIR)
        {
            Directory* subdir = new_Directory(entry, &handle_rw->handle);
            if (subdir == NULL)
            {
                xfree(entry);
                return false;
            }
            bool success = File_traverse_dir(handle_rw, subdir, key_start);
            del_Directory(subdir);
            if (!success)
            {
                xfree(entry);
                return false;
            }
        }
        else if (info == PATH_IS_REGULAR)
        {
            assert((int)strlen(entry) > key_start);
            char* key = entry + key_start;
            long length = path_size(entry, &handle_rw->handle);
            if (length < 0)
            {
                xfree(entry);
                return false;
            }
            char* data = xnalloc(char, length);
            if (data == NULL)
            {
                kqt_Handle_set_error(&handle_rw->handle,
                        "%s: Couldn't allocate memory", __func__);
                xfree(entry);
                return false;
            }
            FILE* in = fopen(entry, "rb");
            if (in == NULL)
            {
                kqt_Handle_set_error(&handle_rw->handle,
                        "%s: Couldnt' open file %s for reading", __func__, entry);
                xfree(entry);
                xfree(data);
                return false;
            }
            long pos = 0;
            char* location = data;
            while (pos < length)
            {
                size_t read = fread(location, 1, MIN(1024, length - pos), in);
                pos += read;
                location += read;
                if (read < 1024 && pos < length)
                {
                    kqt_Handle_set_error(&handle_rw->handle, "%s: Couldn't read"
                            " data from %s", __func__, entry);
                    fclose(in);
                    xfree(entry);
                    xfree(data);
                    return false;
                }
            }
            fclose(in);
            bool success = parse_data(&handle_rw->handle, key, data, length);
            xfree(data);
            if (!success)
            {
                xfree(entry);
                return false;
            }
        }
        xfree(entry);
        entry = Directory_get_entry(dir);
    }
    if (kqt_Handle_get_error(&handle_rw->handle)[0] != '\0')
    {
        return false;
    }
    return true;
}


