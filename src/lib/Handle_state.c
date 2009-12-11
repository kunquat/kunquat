

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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include <Handle_private.h>

#include <kunquat/limits.h>


/**
 * Checks and validates the existence of composition state directories inside
 * a given path.
 *
 * The function sets the generic Handle error if anything goes wrong.
 *
 * \param path            The path to be inspected -- must not be \c NULL.
 * \param has_committed   Return address for the existence of the
 *                        "committed" directory -- must not be \c NULL.
 * \param has_workspace   Return address for the existence of the
 *                        "workspace" directory -- must not be \c NULL.
 * \param has_oldcommit   Return address for the existence of the
 *                        "oldcommit" directory -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool inspect_dirs(const char* path,
                         bool* has_committed,
                         bool* has_workspace,
                         bool* has_oldcommit);


/**
 * Performs a partial recovery inside a given path.
 *
 * The function sets the generic Handle error if anything goes wrong.
 *
 * \param path   The path to be recovered -- must not be \c NULL and must be
 *               in a consistent state.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool partial_recovery(const char* path);


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
 * Removes an entire directory subtree.
 *
 * \param path   The path -- must not be \c NULL and must be a directory.
 *
 * \return   \c true if successful, or \c false if failed. The path is
 *           most likely partially destroyed in case of an error.
 */
static bool remove_tree(const char* path);


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

    bool has_committed = false;
    bool has_workspace = false;
    bool has_oldcommit = false;
    if (!inspect_dirs(path, &has_committed, &has_workspace, &has_oldcommit))
    {
        return NULL;
    }

    if ((has_committed && has_workspace && has_oldcommit)
            || (!has_committed && has_workspace && !has_oldcommit)
            || (!has_committed && !has_workspace && has_oldcommit))
    {
        kqt_Handle_set_error(NULL,
                __func__ ": %s is in an inconsistent state", path);
        return NULL;
    }
    else if ((has_committed && has_workspace && !has_oldcommit)
            || (has_committed && !has_workspace && has_oldcommit)
            || (!has_committed && has_workspace && has_oldcommit))
    {
        if (!partial_recovery(path))
        {
            kqt_Handle_set_error(NULL,
                    __func__ ": Partial recovery of %s failed", path);
            return NULL;
        }
        return kqt_state_init(buffer_size, path);
    }
    else if (!has_committed && !has_workspace && !has_oldcommit)
    {
        char* committed_path = append_to_path(path, "committed");
        if (committed_path == NULL)
        {
            kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
            return NULL;
        }
        errno = 0;
        err = mkdir(committed_path, 0777); // TODO: mode?
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

/*
 * define recovery()
 * committed  workspace  oldcommit
 *  *          *          *          assert false                   # beyond repair
 *  *          *                     delete workspace               # full heal
 *  *                     *          delete oldcommit               # full heal
 *  *                                assert false                   # already correct
 *             *          *          rename workspace to committed  # partial heal
 *             *                     assert false                   # beyond repair
 *                        *          assert false                   # beyond repair
 *                                   assert false                   # already correct
 */

static bool partial_recovery(const char* path)
{
    assert(path != NULL);

    bool has_committed = false;
    bool has_workspace = false;
    bool has_oldcommit = false;
    if (!inspect_dirs(path, &has_committed, &has_workspace, &has_oldcommit))
    {
        return false;
    }
    assert(!(has_committed && has_workspace && has_oldcommit));
    assert(!(has_committed && !has_workspace && !has_oldcommit));
    assert(!(!has_committed && has_workspace && !has_oldcommit));
    assert(!(!has_committed && !has_workspace && has_oldcommit));
    assert(has_committed || has_workspace || has_oldcommit);

    if (has_committed && has_workspace && !has_oldcommit)
    {
        // TODO: delete workspace
    }
    else if (has_committed && !has_workspace && has_oldcommit)
    {
        // TODO: delete oldcommit
    }
    assert(!has_committed && has_workspace && has_oldcommit);

    // rename workspace to committed
    char* workspace_path = append_to_path(path, "workspace");
    if (workspace_path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
        return false;
    }
    char* committed_path = append_to_path(path, "committed");
    if (committed_path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
        xfree(workspace_path);
        return false;
    }
    errno = 0;
    int err = rename(workspace_path, committed_path);
    xfree(workspace_path);
    xfree(committed_path);
    if (err != 0)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't change the \"workspace\""
                " directory into the new \"committed\" directory in %s", path);
        return false;
    }
    return true;
}


static bool inspect_dirs(const char* path,
                         bool* has_committed,
                         bool* has_workspace,
                         bool* has_oldcommit)
{
    assert(path != NULL);
    assert(has_committed != NULL);
    assert(has_workspace != NULL);
    assert(has_oldcommit != NULL);

    errno = 0;
    DIR* ds = opendir(path);
    if (ds == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": %s", strerror(errno));
        return false;
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
        return false;
    }
    
    *has_committed = false;
    *has_workspace = false;
    *has_oldcommit = false;
    while (ret != NULL)
    {
        char* entry = NULL;
        if (strcmp(de->d_name, "committed") == 0)
        {
            entry = "committed";
            *has_committed = true;
        }
        else if (strcmp(de->d_name, "workspace") == 0)
        {
            entry = "workspace";
            *has_workspace = true;
        }
        else if (strcmp(de->d_name, "oldcommit") == 0)
        {
            entry = "oldcommit";
            *has_oldcommit = true;
        }
        if (entry != NULL)
        {
            char* entry_path = append_to_path(path, entry);
            if (entry_path == NULL)
            {
                kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
                closedir(ds);
                return false;
            }
            struct stat* entry_info = &(struct stat){ .st_mode = 0 };
            errno = 0;
            err = stat(entry_path, entry_info);
            xfree(entry_path);
            if (err < 0)
            {
                kqt_Handle_set_error(NULL, __func__ ": %s", strerror(errno));
                closedir(ds);
                return false;
            }
            if (!S_ISDIR(entry_info->st_mode))
            {
                kqt_Handle_set_error(NULL, __func__ ": File %s exists inside %s"
                        " but is not a directory", entry, path);
                closedir(ds);
                return false;
            }
        }
        err = readdir_r(ds, de, &ret);
        if (err != 0)
        {
            kqt_Handle_set_error(NULL, __func__ ": %s", strerror(err));
            closedir(ds);
            return false;
        }
    }
    closedir(ds);
    return true;
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


static bool remove_tree(const char* path)
{
    assert(path != NULL);
}


int kqt_Handle_commit(kqt_Handle* handle)
{
}


