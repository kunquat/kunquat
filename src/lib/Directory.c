

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


#define _POSIX_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#include <math_common.h>
#include <Directory.h>
#include <kunquat/Handle.h>

#include <xmemory.h>


struct Directory
{
    kqt_Handle* handle;
    char* path;
    DIR* dir;
    struct dirent* entry;
};


/**
 * Appends a new file name into a path.
 *
 * \param path   The path -- must not be \c NULL.
 * \param name   The file name -- must not be \c NULL.
 *
 * \return   The new path if successful. or \c NULL if memory allocation
 *           failed. The caller must eventually free the returned string
 *           using xfree.
 */
static char* append_to_path(const char* path, const char* name);


/**
 * Gets the last element of the path.
 *
 * \param path   The path -- must not be \c NULL.
 *
 * \return   The last element.
 */
static char* last_element(const char* path);


/**
 * Copies a regular file.
 *
 * \param dest     The destination path -- must not be \c NULL or an empty
 *                 string.
 * \param src      The source path -- must not be \c NULL, an empty string or
 *                 the same as \a dest.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool copy_file(const char* dest, const char* src, kqt_Handle* handle);


bool create_dir(const char* path, kqt_Handle* handle)
{
    assert(path != NULL);
    assert(path[0] != '\0');
    int err = mkdir(path, 0777); // TODO: mode
    if (err != 0)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't create the directory"
                " %s: %s", path, strerror(errno));
        return false;
    }
    return true;
}


Path_type path_info(const char* path, kqt_Handle* handle)
{
    assert(path != NULL);
    assert(path[0] != '\0');
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) != 0)
    {
        if (errno == ENOENT)
        {
            errno = 0;
            return PATH_NOT_EXIST;
        }
        kqt_Handle_set_error(handle, __func__ ": Couldn't retrieve information"
                " about path %s: %s", path, strerror(errno));
        return PATH_ERROR;
    }
    if (S_ISREG(info->st_mode))
    {
        return PATH_IS_REGULAR;
    }
    else if (S_ISDIR(info->st_mode))
    {
        return PATH_IS_DIR;
    }
    return PATH_IS_OTHER;
}


bool copy_dir(const char* dest, const char* src, kqt_Handle* handle)
{
    assert(dest != NULL);
    assert(dest[0] != '\0');
    assert(src != NULL);
    assert(src[0] != '\0');
    assert(strcmp(dest, src) != 0);
    Directory* src_dir = new_Directory(src, handle);
    if (src_dir == NULL)
    {
        return false;
    }
    if (!create_dir(dest, handle))
    {
        del_Directory(src_dir);
        return false;
    }
    char* subpath = Directory_get_entry(dir);
    while (subpath != NULL)
    {
        bool failed = false;
        Path_type info = path_info(subpath, handle);
        if (info == PATH_ERROR)
        {
            failed = true;
        }
        else if (info == PATH_IS_REGULAR)
        {
            char* dest_subpath = append_to_path(dest, last_element(subpath));
            if (dest_subpath == NULL)
            {
                kqt_Handle_set_error(handle, __func__ ": Couldn't allocate memory");
                failed = true;
            }
            else if (!copy_file(dest_subpath, subpath, handle))
            {
                failed = true;
            }
            xfree(dest_subpath);
        }
        else if (info == PATH_IS_DIR)
        {
            char* dest_subpath = append_to_path(dest, last_element(subpath));
            if (dest_subpath == NULL)
            {
                kqt_Handle_set_error(handle, __func__ ": Couldn't allocate memory");
                failed = true;
            }
            else if (!copy_dir(dest_subpath, subpath, handle))
            {
                failed = true;
            }
            xfree(dest_subpath);
        }
        else if (info == PATH_IS_OTHER)
        {
            kqt_Handle_set_error(handle, __func__ ": Path %s is a foreign"
                    " type of file", subpath);
            failed = true;
        }
        xfree(subpath);
        if (failed)
        {
            del_Directory(src_dir);
            return false;
        }
        subpath = Directory_get_entry(dir);
    }
    del_Directory(dir);
    if (kqt_Handle_get_error(handle)[0] != '\0')
    {
        return false;
    }
    return true;
}


bool move_dir(const char* dest, const char* src, kqt_Handle* handle)
{
    assert(dest != NULL);
    assert(dest[0] != '\0');
    assert(src != NULL);
    assert(src[0] != '\0');
    if (strcmp(dest, src) == 0)
    {
        return true;
    }
    Path_type info = path_info(src, handle);
    if (info == PATH_ERROR)
    {
        return false;
    }
    else if (info != PATH_IS_DIR)
    {
        kqt_Handle_set_error(handle, __func__ ": Source path %s is not"
                " a directory", src);
        return false;
    }
    errno = 0;
    if (rename(src, dest) != 0)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't move the directory"
                " %s into %s: %s", src, dest, strerror(errno));
        return false;
    }
    return true;
}


bool remove_dir(const char* path, kqt_Handle* handle)
{
    assert(path != NULL);
    assert(path[0] != '\0');
    Directory* dir = new_Directory(path, handle);
    if (dir == NULL)
    {
        return false;
    }
    char* subpath = Directory_get_entry(dir);
    while (subpath != NULL)
    {
        bool failed = false;
        Path_type info = path_info(subpath, handle);
        if (info == PATH_ERROR)
        {
            failed = true;
        }
        else if (info == PATH_IS_REGULAR)
        {
            errno = 0;
            if (remove(subpath) != 0)
            {
                kqt_Handle_set_error(handle, __func__ ": Couldn't remove file"
                        " %s: %s", subpath, strerror(errno));
                failed = true;
            }
        }
        else if (info == PATH_IS_DIR)
        {
            if (!remove_dir(subpath, handle))
            {
                failed = true;
            }
        }
        else if (info == PATH_IS_OTHER)
        {
            kqt_Handle_set_error(handle, __func__ ": Path %s contains an"
                    " unexpected type of file %s", path, subpath);
            failed = true;
        }
        xfree(subpath);
        if (failed)
        {
            del_Directory(dir);
            return false;
        }
        subpath = Directory_get_entry(dir);
    }
    del_Directory(dir);
    if (kqt_Handle_get_error(handle)[0] != '\0')
    {
        return false;
    }
    errno = 0;
    if (remove(path) != 0)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't remove directory"
                " %s: %s", path, strerror(errno));
        return false;
    }
    return true;
}


Directory* new_Directory(const char* path, kqt_Handle* handle)
{
    assert(path != NULL);
    assert(path[0] != '\0');
    Directory* dir = xalloc(Directory);
    if (dir == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't allocate memory");
        return NULL;
    }
    dir->handle = handle;
    dir->path = NULL;
    dir->dir = NULL;
    dir->entry = NULL;

    int path_len = strlen(path);
    bool trailing_slash = path_len > 0 && path[path_len - 1] == '/';
    dir->path = xcalloc(char, path_len + (trailing_slash ? 0 : 1) + 1);
    if (dir->path == NULL)
    {
        kqt_Handle_set_error(dir->handle, __func__ ": Couldn't allocate memory");
        del_Directory(dir);
        return NULL;
    }
    strcpy(dir->path, path);
    if (!trailing_slash)
    {
        strcat(dir->path, "/");
    }

    errno = 0;
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        kqt_Handle_set_error(dir->handle, __func__ ": Couldn't open path %s: %s",
                path, strerror(errno));
        del_Directory(dir);
        return NULL;
    }
    long path_len_max = fpathconf(fd, _PC_NAME_MAX);
    path_len_max = MAX(path_len_max, 512);
    errno = 0;
    dir->dir = fdopendir(fd);
    if (dir->dir == NULL)
    {
        kqt_Handle_set_error(dir->handle, __func__ ": Couldn't open path %s as a directory: %s",
                path, strerror(errno));
        close(fd);
        del_Directory(dir);
        return NULL;
    }

    dir->entry = xcalloc(char, offsetof(struct dirent, d_name) + path_len_max + 1);
    if (dir->entry == NULL)
    {
        kqt_Handle_set_error(dir->handle, __func__ ": Couldn't allocate memory");
        del_Directory(dir);
        return NULL;
    }

    return dir;
}


char* Directory_get_entry(Directory* dir)
{
    assert(dir != NULL);
    struct dirent* ret = NULL;
    do
    {
        int err = readdir_r(dir->dir, dir->entry, &ret);
        if (err != 0)
        {
            kqt_Handle_set_error(dir->handle, __func__ ": Couldn't retrieve a directory"
                    " entry in %s: %s", dir->path, strerror(err));
            return NULL;
        }
        if (ret == NULL)
        {
            return NULL;
        }
    } while (strcmp(dir->entry->d_name, ".") == 0 || strcmp(dir->entry->d_name, "..") == 0);
    char* sub_path = append_to_path(dir->path, dir->entry->d_name);
    if (sub_path == NULL)
    {
        kqt_Handle_set_error(dir->handle, __func__ ": Couldn't allocate memory");
        return NULL;
    }
    return sub_path;
}


void del_Directory(Directory* dir)
{
    assert(dir != NULL);
    if (dir->path != NULL)
    {
        xfree(path);
    }
    if (dir->dir != NULL)
    {
        closedir(dir->dir);
    }
    if (dir->entry != NULL)
    {
        xfree(dir->entry);
    }
    xfree(dir);
    return;
}


static char* append_to_path(const char* path, const char* name)
{
    assert(path != NULL);
    assert(name != NULL);
    int path_len = strlen(path);
    bool trailing_slash = path_len > 0 && path[path_len - 1] == '/';
    char* new_path = xcalloc(char, path_len +
            (trailing_slash ? 0 : 1) + strlen(name) + 1);
    if (new_path == NULL)
    {
        return NULL;
    }
    strcpy(new_path, path);
    if (!trailing_slash)
    {
        strcat(new_path, "/");
    }
    strcat(new_path, name);
    return new_path;
}


static char* last_element(const char* path)
{
    assert(path != NULL);
    bool non_solidus_found = false;
    int index = 0;
    for (index = strlen(path) - 1; index >= 0; --i)
    {
        if (!non_solidus_found && path[i] != '/')
        {
            non_solidus_found = true;
        }
        else if (non_solidus_found && path[i] == '/')
        {
            break;
        }
    }
    assert(non_solidus_found);
    ++index;
    assert(index >= 0);
    return &path[index];
}


static bool copy_file(const char* dest, const char* src, kqt_Handle* handle)
{
    assert(dest != NULL);
    assert(dest[0] != '\0');
    assert(src != NULL);
    assert(src[0] != '\0');
    assert(strcmp(dest, src) != 0);

    errno = 0;
    FILE* in = fopen(src, "rb");
    if (in == NULL)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't open the copy"
                " source %s: %s", src, strerror(errno));
        return false;
    }
    FILE* out = fopen(dest, "rb");
    if (out != NULL)
    {
        fclose(in);
        fclose(out);
        kqt_Handle_set_error(handle, __func__ ": The copy destination %s"
                " already exists", dest);
        return false;
    }
    errno = 0;
    out = fopen(dest, "wb");
    if (out == NULL)
    {
        fclose(in);
        kqt_Handle_set_error(handle, __func__ ": Couldn't open the copy"
                " destination %s: %s", dest, strerror(errno));
        return false;
    }

    char buf[1024] = { 0 };
    size_t in_bytes = fread(buf, 1, 1024, in);
    while (in_bytes > 0)
    {
        size_t out_bytes = fwrite(buf, 1, 1024, out);
        if (out_bytes < in_bytes)
        {
            kqt_Handle_set_error(handle, __func__ ": Couldn't write into"
                    " the destination file %s");
            fclose(in);
            fclose(out);
            return false;
        }
        in_bytes = fread(buf, 1, 1024, in);
    }
    if (ferror(in))
    {
        kqt_Handle_set_error(handle, __func__ ": An error occurred while"
                " reading the input file %s", src);
        fclose(in);
        fclose(out);
        return false;
    }

    errno = 0;
    fclose(in);
    if (fclose(out) == EOF)
    {
        kqt_Handle_set_error(handle, __func__ ": Couldn't close the copy"
                " destination %s: %s", dest, strerror(errno));
        return false;
    }
    return true;
}


