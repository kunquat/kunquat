

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <Directory.h>
#include <Handle_rwc.h>
#include <File_dir.h>

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
 * Sets data into a read/write/commit Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- must be valid and support commits.
 */
static int Handle_rwc_set_data(kqt_Handle* handle,
                               const char* key,
                               void* data,
                               long length);


/**
 * Destroys an existing read/write/commit Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL and must be a
 *                 read/write/commit Handle.
 */
static void del_Handle_rwc(kqt_Handle* handle);


/*
 * define init()
 * committed  workspace  oldcommit
 *  *          *          *          throw exception
 *  *          *                     recovery() && return init()
 *  *                     *          recovery() && return init()
 *  *                                copy committed workspace && return handle
 *             *          *          recovery() && return init()
 *             *                     throw exception
 *                        *          throw exception
 *                                   mkdir committed && return init()
 */
kqt_Handle* kqt_new_Handle_rwc(long buffer_size, char* path)
{
    if (buffer_size <= 0)
    {
        kqt_Handle_set_error(NULL, ERROR_ARGUMENT,
                "Buffer size must be positive");
        return NULL;
    }
    if (path == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_ARGUMENT, "No input path given");
        return NULL;
    }

    Path_type info = path_info(path, NULL);
    if (info == PATH_ERROR)
    {
        return NULL;
    }
    else if (info == PATH_NO_ENTRY)
    {
        kqt_Handle_set_error(NULL, ERROR_FORMAT, "Path %s does not exist", path);
        return NULL;
    }
    else if (info != PATH_IS_DIR)
    {
        kqt_Handle_set_error(NULL, ERROR_FORMAT,
                "Path %s is not a directory", path);
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
        kqt_Handle_set_error(NULL, ERROR_FORMAT, "Project directory %s is"
                " in an inconsistent state", path);
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
        return kqt_new_Handle_rwc(buffer_size, path);
    }
    else if (!has_committed && !has_workspace && !has_oldcommit)
    {
        char* committed_path = append_to_path(path, "committed");
        if (committed_path == NULL)
        {
            kqt_Handle_set_error(NULL, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return NULL;
        }
        if (!create_dir(committed_path, NULL))
        {
            xfree(committed_path);
            return NULL;
        }
        char* dir_root_path = append_to_path(committed_path,
                MAGIC_ID "c" KQT_FORMAT_VERSION);
        xfree(committed_path);
        if (dir_root_path == NULL)
        {
            kqt_Handle_set_error(NULL, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return NULL;
        }
        bool created = create_dir(dir_root_path, NULL);
        xfree(dir_root_path);
        if (!created)
        {
            return NULL;
        }
        return kqt_new_Handle_rwc(buffer_size, path);
    }

    assert(has_committed && !has_workspace && !has_oldcommit);

    char* committed_path = append_to_path(path, "committed");
    if (committed_path == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        return NULL;
    }
    char* workspace_path = append_to_path(path, "workspace");
    if (workspace_path == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        xfree(committed_path);
        return NULL;
    }
    bool copied = copy_dir(workspace_path, committed_path, NULL);
    xfree(committed_path);
    if (!copied)
    {
        xfree(workspace_path);
        return NULL;
    }
    char* abs_workspace = absolute_path(workspace_path, NULL);
    xfree(workspace_path);
    if (abs_workspace == NULL)
    {
        return NULL;
    }

    Handle_rwc* handle_rwc = xalloc(Handle_rwc);
    if (handle_rwc == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        xfree(abs_workspace);
        return NULL;
    }
    handle_rwc->handle_rw.handle.mode = KQT_READ_WRITE_COMMIT;
    handle_rwc->handle_rw.base_path = append_to_path(abs_workspace,
            "kunquatc" KQT_FORMAT_VERSION);
    xfree(abs_workspace);
    if (handle_rwc->handle_rw.base_path == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        del_Handle_rwc(&handle_rwc->handle_rw.handle);
        return NULL;
    }
    handle_rwc->changed_files = new_AAtree((int (*)(const void*, const void*))strcmp, free);
    if (handle_rwc->changed_files == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        del_Handle_rwc(&handle_rwc->handle_rw.handle);
        return NULL;
    }

    if (!kqt_Handle_init(&handle_rwc->handle_rw.handle, buffer_size))
    {
        del_Handle_rwc(&handle_rwc->handle_rw.handle);
        return NULL;
    }
    handle_rwc->handle_rw.handle.mode = KQT_READ_WRITE_COMMIT;
    handle_rwc->handle_rw.handle.get_data = Handle_rw_get_data;
    handle_rwc->handle_rw.handle.get_data_length = Handle_rw_get_data_length;
    handle_rwc->handle_rw.handle.destroy = del_Handle_rwc;
    handle_rwc->handle_rw.set_data = Handle_rwc_set_data;
    if (!File_dir_open(&handle_rwc->handle_rw, handle_rwc->handle_rw.base_path))
    {
        kqt_del_Handle(&handle_rwc->handle_rw.handle);
        return NULL;
    }
    return &handle_rwc->handle_rw.handle;
}


int kqt_Handle_commit(kqt_Handle* handle)
{
    check_handle(handle, 0);
    if (handle->mode != KQT_READ_WRITE_COMMIT)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Cannot commit changes"
                " in a non-commit Kunquat Handle");
        return 0;
    }
    Handle_rwc* handle_rwc = (Handle_rwc*)handle;
    assert(handle_rwc->changed_files != NULL);
    int path_len = strlen(handle_rwc->handle_rw.base_path);

    // switch committed and workspace
    char* const committed_path = xcalloc(char, path_len + 1);
    if (committed_path == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY, "Couldn't allocate memory");
        return 0;
    }
    char* const oldcommit_path = xcalloc(char, path_len + 1);
    if (oldcommit_path == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY, "Couldn't allocate memory");
        xfree(committed_path);
        return 0;
    }
    strcpy(committed_path, handle_rwc->handle_rw.base_path);
    strcpy(oldcommit_path, handle_rwc->handle_rw.base_path);
    char* workspace_pos = strstr(committed_path, "workspace");
    char* prev_workspace_pos = NULL;
    while (workspace_pos != NULL)
    {
        prev_workspace_pos = workspace_pos;
        workspace_pos = strstr(workspace_pos + 1, "workspace");
    }
    assert(prev_workspace_pos != NULL);
    int workspace_index = prev_workspace_pos - committed_path;
    strcpy(committed_path + workspace_index, "committed");
    strcpy(oldcommit_path + workspace_index, "oldcommit");
    bool moved = move_dir(oldcommit_path, committed_path, handle);
    char* workspace_path = oldcommit_path;
    strcpy(workspace_path + workspace_index, "workspace");
    if (moved) moved = move_dir(committed_path, workspace_path, handle);
    workspace_path = committed_path;
    strcpy(workspace_path + workspace_index, "workspace");
    strcpy(oldcommit_path + workspace_index, "oldcommit");
    if (moved) moved = move_dir(workspace_path, oldcommit_path, handle);
    xfree(committed_path);
    xfree(oldcommit_path);
    if (!moved)
    {
        return 0;
    }

    // update workspace
    AAtree* pending_removal = NULL;
    char* target_key = AAtree_get(handle_rwc->changed_files, "");
    while (target_key != NULL)
    {
        char* target_in_committed = append_to_path(handle_rwc->handle_rw.base_path,
                target_key);
        char* target_in_workspace = append_to_path(handle_rwc->handle_rw.base_path,
                target_key);
        if (target_in_committed == NULL || target_in_workspace == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            if (pending_removal != NULL) del_AAtree(pending_removal);
            xfree(target_in_committed);
            xfree(target_in_workspace);
            return 0;
        }
        strncpy(target_in_committed + workspace_index, "committed", 9);
        char* xx = strstr(target_in_committed, "XX");
        while (xx != NULL)
        {
            strncpy(xx, KQT_FORMAT_VERSION, 2);
            xx = strstr(xx + 1, "XX");
        }
        xx = strstr(target_in_workspace, "XX");
        while (xx != NULL)
        {
            strncpy(xx, KQT_FORMAT_VERSION, 2);
            xx = strstr(xx + 1, "XX");
        }
        Path_type committed_info = path_info(target_in_committed, handle);
        if (committed_info == PATH_ERROR)
        {
            if (pending_removal != NULL) del_AAtree(pending_removal);
            xfree(target_in_committed);
            xfree(target_in_workspace);
            return 0;
        }
        if (committed_info == PATH_NO_ENTRY)
        {
            if (pending_removal == NULL)
            {
                pending_removal = new_AAtree(
                        (int (*)(const void*, const void*))strcmp, free);
                if (pending_removal == NULL)
                {
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                    xfree(target_in_committed);
                    xfree(target_in_workspace);
                    return 0;
                }
            }
            if (!AAtree_ins(pending_removal, target_in_workspace))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_AAtree(pending_removal);
                xfree(target_in_committed);
                xfree(target_in_workspace);
                return 0;
            }
            xfree(target_in_committed);
        }
        else if (committed_info != PATH_IS_OTHER)
        {
            bool copied = false;
            if (committed_info == PATH_IS_REGULAR)
            {
                copied = copy_file(target_in_workspace, target_in_committed,
                                   handle);
            }
            else
            {
                assert(committed_info == PATH_IS_DIR);
                copied = copy_dir(target_in_workspace, target_in_committed,
                                  handle);
            }
            xfree(target_in_committed);
            xfree(target_in_workspace);
            if (!copied)
            {
                if (pending_removal != NULL) del_AAtree(pending_removal);
                return 0;
            }
        }
        else
        {
            kqt_Handle_set_error(handle, ERROR_FORMAT,
                    "File %s has a foreign type", target_in_committed);
            if (pending_removal != NULL) del_AAtree(pending_removal);
            xfree(target_in_committed);
            xfree(target_in_workspace);
            return 0;
        }

        char* entry_to_remove = AAtree_remove(handle_rwc->changed_files, target_key);
        assert(entry_to_remove == target_key);
        (void)entry_to_remove;
        xfree(target_key);
        target_key = AAtree_get(handle_rwc->changed_files, "");
    }
    if (pending_removal != NULL)
    {
        // remove files in reverse alphabetical order
        char* target_file = AAtree_get_at_most(pending_removal, "~~~~");
        while (target_file != NULL)
        {
            errno = 0;
            if (remove(target_file) != 0)
            {
                kqt_Handle_set_error(handle, ERROR_RESOURCE, "Couldn't remove"
                        " %s: %s", target_file, strerror(errno));
                del_AAtree(pending_removal);
                return 0;
            }
            char* file_to_remove = AAtree_remove(pending_removal, target_file);
            assert(file_to_remove == target_file);
            (void)file_to_remove;
            xfree(target_file);
            target_file = AAtree_get(pending_removal, "~~~~");
        }
        del_AAtree(pending_removal);
    }
    return 1;
}


static int Handle_rwc_set_data(kqt_Handle* handle,
                               const char* key,
                               void* data,
                               long length)
{
    assert(handle != NULL);
    assert(handle->mode == KQT_READ_WRITE_COMMIT);
    assert(key_is_valid(handle, key));
    Handle_rwc* handle_rwc = (Handle_rwc*)handle;
    bool new_file_changed = false;
    if (AAtree_get_exact(handle_rwc->changed_files, key) == NULL)
    {
        new_file_changed = true;
        char* key_copy = xcalloc(char, strlen(key) + 1);
        if (key_copy == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't log changes to the key %s", key);
            return 0;
        }
        strcpy(key_copy, key);
        if (!AAtree_ins(handle_rwc->changed_files, key_copy))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't log changes to the key %s", key);
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
            kqt_Handle_set_error(NULL, ERROR_MEMORY,
                    "Couldn't allocate memory");
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
            kqt_Handle_set_error(NULL, ERROR_MEMORY,
                    "Couldn't allocate memory");
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
        kqt_Handle_set_error(NULL, ERROR_MEMORY,
                "Couldn't allocate memory");
        return false;
    }
    char* committed_path = append_to_path(path, "committed");
    if (committed_path == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY,
                "Couldn't allocate memory");
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
        if (strcmp(last_element(entry), "committed") == 0)
        {
            found = true;
            *has_committed = true;
        }
        else if (strcmp(last_element(entry), "workspace") == 0)
        {
            found = true;
            *has_workspace = true;
        }
        else if (strcmp(last_element(entry), "oldcommit") == 0)
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
                kqt_Handle_set_error(NULL, ERROR_FORMAT, "File %s exists"
                        " inside %s but is not a directory", entry, path);
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
    assert(handle != NULL);
    assert(handle->mode == KQT_READ_WRITE_COMMIT);
    Handle_rwc* handle_rwc = (Handle_rwc*)handle;
    if (handle_rwc->handle_rw.base_path != NULL)
    {
        xfree(handle_rwc->handle_rw.base_path);
    }
    if (handle_rwc->changed_files != NULL)
    {
        del_AAtree(handle_rwc->changed_files);
    }
    xfree(handle_rwc);
    return;
}


