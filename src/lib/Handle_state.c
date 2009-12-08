

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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include <Handle_private.h>

#include <kunquat/limits.h>


static void recovery(char* path);


kqt_Handle* kqt_state_init(long buffer_size, char* path)
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

    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        kqt_Handle_set_error(NULL,
                __func__ ": Couldn't access %s: %s", path, strerror(errno));
        return NULL;
    }
    if (!S_ISDIR(info->st_mode))
    {
        kqt_Handle_set_error(NULL, __func__ ": path is not a directory");
        return NULL;
    }

    errno = 0;
    DIR* ds = opendir(path);
    if (ds == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": %s", strerror(errno));
        return NULL;
    }
    union
    {
        struct dirent d;
        char buf[offsetof(struct dirent, d_name) + _PC_NAME_MAX + 1];
    } entry_area;
    struct dirent* de = &entry_area.d;
    struct dirent* ret = NULL;
    int err = readdir_r(ds, de, &ret);
    if (err != 0)
    {
        kqt_Handle_set_error(NULL, __func__ ": %s", strerror(err));
        closedir(ds);
        return NULL;
    }
    bool has_committed = false;
    bool has_workspace = false;
    bool has_oldcommit = false;
    while (ret != NULL)
    {
        char* entry = NULL;
        if (strcmp(de->d_name, "committed") == 0)
        {
            entry = "committed";
            has_committed = true;
        }
        else if (strcmp(de->d_name, "workspace") == 0)
        {
            entry = "workspace";
            has_workspace = true;
        }
        else if (strcmp(de->d_name, "oldcommit") == 0)
        {
            entry = "oldcommit";
            has_oldcommit = true;
        }
        if (entry != NULL)
        {
            char* entry_path = xcalloc(char, strlen(path) + 1 + strlen(entry) + 1);
            if (entry_path == NULL)
            {
                kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
                closedir(ds);
                return NULL;
            }
            strcpy(entry_path, path);
            strcat(entry_path, "/");
            strcat(entry_path, entry);
            struct stat* entry_info = &(struct stat){ .st_mode = 0 };
            errno = 0;
            if (stat(entry_path, entry_info) < 0)
            {
                kqt_Handle_set_error(NULL, __func__ ": %s", strerror(errno));
                closedir(ds);
                return NULL;
            }
            if (!S_ISDIR(entry_info->st_mode))
            {
                kqt_Handle_set_error(NULL, __func__ ": File %s exists inside %s"
                        " but is not a directory", entry, path);
                closedir(ds);
                return NULL;
            }
            xfree(entry_path);
        }
        err = readdir_r(ds, de, &ret);
        if (err != 0)
        {
            kqt_Handle_set_error(NULL, __func__ ": %s", strerror(err));
            closedir(ds);
            return NULL;
        }
    }
    closedir(ds);

    if ((has_committed && has_workspace && has_oldcommit)
            || (!has_committed && has_workspace && !has_oldcommit)
            || (!has_committed && !has_workspace && !has_oldcommit))
    {
        kqt_Handle_set_error(NULL,
                __func__ ": %s is in an inconsistent state", path);
        return NULL;
    }
    else if ((has_committed && has_workspace && !has_oldcommit)
            || (has_committed && !has_workspace && has_oldcommit)
            || (!has_committed && has_workspace && has_oldcommit))
    {
        recovery(path);
        return kqt_state_init(buffer_size, path);
    }
    else if (!has_committed && !has_workspace && !has_oldcommit)
    {
        char* committed_path = xcalloc(char, strlen(path) + 1 + strlen("committed") + 1);
        if (committed_path == NULL)
        {
            kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
            return NULL;
        }
        strcpy(committed_path, path);
        strcat(committed_path, "/committed");
        errno = 0;
        err = mkdir(committed_path, 0777); // FIXME: mode?
        xfree(committed_path);
        if (err != 0)
        {
            kqt_Handle_set_error(NULL, __func__ ": Couldn't create the \"committed\""
                    " directory inside %s: %s", path, strerror(errno));
            return NULL;
        }
        return kqt_state_init(buffer_size, path);
    }

    assert(has_committed && !has_workspace && !has_oldcommit);

    kqt_Handle* handle = kqt_new_Handle(buffer_size);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL,
                __func__ ": Couldn't allocate memory for a new Kunquat Handle");
        return NULL;
    }
}


static void recovery(char* path)
{
    assert(path != NULL);
}


int kqt_Handle_commit(kqt_Handle* handle)
{
}


