

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

#include <File_tree.h>
#include <Directory.h>
#include <Handle_private.h>

#include <xmemory.h>

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
 * Destroys an existing read/write/commit Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL and must be a
 *                 read/write/commit Handle.
 */
static void del_Handle_rwc(kqt_Handle* handle);


kqt_Handle* kqt_new_Handle_rwc(long buffer_size, char* path)
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
        bool created = create_dir(committed_path, NULL);
        xfree(committed_path);
        if (!created)
        {
            kqt_Handle_set_error(NULL, __func__ ": Couldn't create the \"committed\""
                    " directory inside %s: %s", path, strerror(errno));
            return NULL;
        }
        return kqt_state_init(buffer_size, path);
    }

    assert(has_committed && !has_workspace && !has_oldcommit);

    char* committed_path = append_to_path(path, "committed");
    if (committed_path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
        return NULL;
    }
    char* workspace_path = append_to_path(path, "workspace");
    if (workspace_path == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
        xfree(committed_path);
        return NULL;
    }
    bool copied = copy_dir(workspace_path, committed_path, NULL);
    xfree(committed_path);
    if (!copied)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't copy the \"committed\""
                " directory into the new \"workspace\" directory");
        xfree(workspace_path);
        return NULL;
    }

    Handle_rwc* handle_rwc = xalloc(Handle_rwc);
    if (handle == NULL)
    {
        kqt_Handle_set_error(NULL,
                __func__ ": Couldn't allocate memory for a new Kunquat Handle");
        xfree(workspace_path);
        return NULL;
    }
    handle_rwc->handle_rw.handle.mode = KQT_READ_WRITE_COMMIT;
    handle_rwc->handle_rw.base_path = workspace_path;
    handle_rwc->changed_files = new_AAtree(strcmp, free);
    if (handle_rwc->changed_files == NULL)
    {
        kqt_Handle_set_error(NULL,
                __func__ ": Couldn't allocate memory for a new Kunquat Handle");
        del_Handle_rwc(&handle_rwc->handle_rw.handle);
        return NULL;
    }

    Read_state* state = READ_STATE_AUTO;
    File_tree* tree = new_File_tree_from_fs(handle_rwc->handle_rw.base_path, state);
    if (tree == NULL)
    {
        kqt_Handle_set_error(NULL, __func__ ": Couldn't load the path %s"
                " as a Kunquat composition directory: %s:%d: %s",
                handle_rwc->handle_rw.base_path,
                state->path, state->row, state->message);
        del_Handle_rwc(&handle_rwc->handle_rw.handle);
        return NULL;
    }
    if (!kqt_Handle_init(&handle_rwc->handle_rw.handle, buffer_size, tree))
    {
        del_File_tree(tree);
        del_Handle_rwc(&handle_rwc->handle_rw.handle);
        return NULL;
    }
    del_File_tree(tree);

    handle_rwc->handle_rw.handle.mode = KQT_READ_WRITE_COMMIT;
    handle_rwc->handle_rw.handle.get_data = Handle_rw_get_data;
    handle_rwc->handle_rw.handle.get_data_length = Handle_rw_get_data_length;
    handle_rwc->handle_rw.handle.set_data = Handle_rwc_set_data;
    handle_rwc->handle_rw.handle.destroy = del_Handle_rwc;
    return &handle_rwc->handle_rw.handle;
}


int kqt_Handle_commit(kqt_Handle* handle)
{
    check_handle(handle, __func__, 0);
    if (handle->mode != KQT_READ_WRITE_COMMIT)
    {
        kqt_Handle_set_error(handle, __func__
                ": Cannot commit changes in a non-commit Kunquat Handle");
        return 0;
    }
}


static int Handle_rwc_set_data(kqt_Handle* handle,
                               char* key,
                               void* data,
                               int length)
{
    assert(handle != NULL);
    assert(handle->mode == KQT_READ_WRITE_COMMIT);
    Handle_rwc* handle_rwc = (Handle_rwc*)handle;
    bool new_file_changed = false;
    if (AAtree_get_exact(handle_rwc->changed_files) == NULL)
    {
        new_file_changed = true;
        char* key_copy = xnalloc(char, strlen(key) + 1);
        if (key_copy == NULL)
        {
            kqt_Handle_set_error(handle, __func__
                    ": Couldn't log changes to the key %s", key);
            return 0;
        }
        if (!AAtree_ins(handle_rwc->changed_files, key_copy))
        {
            kqt_Handle_set_error(handle, __func__
                    ": Couldn't log changes to the key %s", key);
            xfree(key_copy);
            return 1;
        }
    }
    int set = Handle_rw_set_data(handle, key, data, length);
    if (!set && new_file_changed)
    {
        char* key_copy = AAtree_remove(handle_rwc->changed_files, key);
        xfree(key_copy);
    }
    return set;
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
        char* workspace_path = append_to_path(path, "workspace");
        if (workspace_path == NULL)
        {
            kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
            return false;
        }
        bool removed = remove_dir(workspace_path, NULL);
        xfree(workspace_path);
        return removed;
    }
    else if (has_committed && !has_workspace && has_oldcommit)
    {
        char* oldcommit_path = append_to_path(path, "oldcommit");
        if (oldcommit_path == NULL)
        {
            kqt_Handle_set_error(NULL, __func__ ": Couldn't allocate memory");
            return false;
        }
        bool removed = remove_dir(oldcommit_path, NULL);
        xfree(oldcommit_path);
        return removed;
    }
    assert(!has_committed && has_workspace && has_oldcommit);

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
    bool moved = move_dir(committed_path, workspace_path, NULL);
    xfree(workspace_path);
    xfree(committed_path);
    return moved;
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

    Directory* dir = new_Directory(path, NULL);
    if (dir == NULL)
    {
        return false;
    }
    
    *has_committed = false;
    *has_workspace = false;
    *has_oldcommit = false;
    char* entry = Directory_get_entry(dir);
    while (entry != NULL)
    {
        bool found = false;
        if (strcmp(entry, "committed") == 0)
        {
            found = true;
            *has_committed = true;
        }
        else if (strcmp(entry, "workspace") == 0)
        {
            found = true;
            *has_workspace = true;
        }
        else if (strcmp(entry, "oldcommit") == 0)
        {
            found = true;
            *has_oldcommit = true;
        }
        if (found)
        {
            Path_type type = path_info(entry, NULL);
            if (type == PATH_ERROR)
            {
                xfree(entry);
                del_Directory(dir);
                return false;
            }
            if (type != PATH_IS_DIR)
            {
                kqt_Handle_set_error(NULL, __func__ ": File %s exists inside %s"
                        " but is not a directory", entry, path);
                xfree(entry);
                del_Directory(dir);
                return false;
            }
        }
        xfree(entry);
        entry = Directory_get_entry(dir);
    }
    del_Directory(dir);
    if (kqt_Handle_get_error(NULL)[0] != '\0')
    {
        return false;
    }
    return true;
}


static void del_Handle_rwc(kqt_Handle* handle)
{
    assert(handle_is_valid(handle));
    assert(handle->mode == KQT_READ_WRITE_COMMIT);
    Handle_rwc* handle_rwc = (Handle_rwc*)handle;
    if (handle_rwc->handle.base_path != NULL)
    {
        xfree(handle_rwc->handle.base_path);
    }
    if (handle_rwc->changed_files != NULL)
    {
        del_AAtree(handle_rwc->changed_files);
    }
    xfree(handle_rwc);
    return;
}


