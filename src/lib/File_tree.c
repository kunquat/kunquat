

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
#include <string.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

#include <AAtree.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


File_tree* new_File_tree(char* name, char* path, char* data)
{
    assert(name != NULL);
    assert(path != NULL);
    File_tree* tree = xalloc(File_tree);
    if (tree == NULL)
    {
        return NULL;
    }
    if (data == NULL)
    {
        AAtree* children = new_AAtree((int (*)(void*, void*))File_tree_cmp,
                                      (void (*)(void*))del_File_tree);
        if (children == NULL)
        {
            xfree(tree);
            return NULL;
        }
        tree->is_dir = true;
        tree->content.children = children;
    }
    else
    {
        tree->is_dir = false;
        tree->content.data = data;
    }
    tree->name = name;
    tree->path = path;
    return tree;
}


File_tree* new_File_tree_from_fs(char* path)
{
    assert(path != NULL);
    struct stat* info = &(struct stat){ .st_mode = 0 };
    errno = 0;
    if (stat(path, info) < 0)
    {
        return NULL;
    }
    int name_start = 0;
    int len = strlen(path);
    char* path_name = xcalloc(char, len + 1);
    if (path_name == NULL)
    {
        return NULL;
    }
    strcpy(path_name, path);
    for (int i = 0; i < len - 1; ++i) // -1: ignore a trailing '/'
    {
        if (path[i] == '/' && path[i + 1] != '/')
        {
            name_start = i + 1;
        }
    }
    char* name = xcalloc(char, (len + 1) - name_start);
    if (name == NULL)
    {
        xfree(path_name);
        return NULL;
    }
    len -= name_start;
    strcpy(name, path + name_start);
    name[len] = '\0';
    for (int i = 0; i < len; ++i)
    {
        if (name[i] == '/')
        {
            name[i] = '\0';
            break;
        }
    }
    File_tree* tree = NULL;
    if (S_ISDIR(info->st_mode))
    {
        tree = new_File_tree(name, path_name, NULL);
        if (tree == NULL)
        {
            xfree(name);
            xfree(path_name);
            return NULL;
        }
        DIR* ds = opendir(path);
        if (ds == NULL)
        {
            del_File_tree(tree);
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
            del_File_tree(tree);
            closedir(ds);
            return NULL;
        }
        while (ret != NULL)
        {
            if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
            {
                len = strlen(path) + strlen(de->d_name);
                char* full_path = xcalloc(char, len + 2);
                if (full_path == NULL)
                {
                    del_File_tree(tree);
                    closedir(ds);
                    return NULL;
                }
                strcpy(full_path, path);
                strcat(full_path, "/");
                strcat(full_path, de->d_name);
                File_tree* child = new_File_tree_from_fs(full_path);
                xfree(full_path);
                if (child == NULL)
                {
                    del_File_tree(tree);
                    closedir(ds);
                    return NULL;
                }
                if (!File_tree_ins_child(tree, child))
                {
                    del_File_tree(child);
                    del_File_tree(tree);
                    closedir(ds);
                    return NULL;
                }
            }
            err = readdir_r(ds, de, &ret);
            if (err != 0)
            {
                del_File_tree(tree);
                closedir(ds);
                return NULL;
            }
        }
        closedir(ds);
    }
    else
    {
        Read_state* state = &(Read_state){ .error = false, .row = 0, .message = { '\0' } };
        FILE* in = fopen(path, "rb");
        if (in == NULL)
        {
            xfree(name);
            xfree(path_name);
            return NULL;
        }
        char* data = read_file(in, state);
        fclose(in);
        if (state->error)
        {
            xfree(name);
            xfree(path_name);
            return NULL;
        }
        assert(data != NULL);
        tree = new_File_tree(name, path_name, data);
        if (tree == NULL)
        {
            xfree(data);
            xfree(name);
            xfree(path_name);
            return NULL;
        }
    }
    assert(tree != NULL);
    return tree;
}


bool File_tree_create_branch(File_tree* tree, const char* path, char* data)
{
    assert(tree != NULL);
    assert(File_tree_is_dir(tree));
    assert(path != NULL);
    if (!File_tree_is_dir(tree))
    {
        return false;
    }
    const char* next_element = strchr(path, '/');
    if (next_element == NULL)
    {
        int cpath_len = strlen(File_tree_get_path(tree)) + strlen(path);
        char* cpath = xcalloc(char, cpath_len + 1);
        if (cpath == NULL)
        {
            return false;
        }
        char* cname = xcalloc(char, strlen(path) + 1);
        if (cname == NULL)
        {
            xfree(cpath);
            return false;
        }
        strcpy(cpath, File_tree_get_path(tree));
        strcat(cpath, path);
        strcpy(cname, path);
        File_tree* child = new_File_tree(cname, cpath, data);
        if (child == NULL)
        {
            xfree(cpath);
            xfree(cname);
            return false;
        }
        if (!File_tree_ins_child(tree, child))
        {
            child->content.data = NULL;
            del_File_tree(child);
            return false;
        }
        return true;
    }

    int cname_len = next_element - path;
    while (*next_element == '/')
    {
        ++next_element;
    }
    char* cname = xcalloc(char, cname_len + 1);
    if (cname == NULL)
    {
        return false;
    }
    strncpy(cname, path, cname_len);
    File_tree* child = File_tree_get_child(tree, cname);
    if (child != NULL)
    {
        xfree(cname);
        if (*next_element == '\0')
        {
            return true;
        }
        return File_tree_create_branch(child, next_element, data);
    }
    int cpath_len = strlen(File_tree_get_path(tree)) + cname_len;
    char* cpath = xcalloc(char, cpath_len + 1);
    if (cpath == NULL)
    {
        xfree(cname);
        return false;
    }
    strcpy(cpath, File_tree_get_path(tree));
    strncat(cpath, path, cname_len);
    child = new_File_tree(cname, cpath, NULL);
    if (child == NULL)
    {
        xfree(cpath);
        xfree(cname);
        return false;
    }
    if (!File_tree_ins_child(tree, child))
    {
        del_File_tree(child);
        return false;
    }
    if (*next_element == '\0')
    {
        return true;
    }
    return File_tree_create_branch(child, next_element, data);
}


#define fail_if(cond, reader, state)                                       \
    do                                                                     \
    {                                                                      \
        if ((cond))                                                        \
        {                                                                  \
            archive_read_finish((reader));                                 \
            Read_state_set_error((state), archive_error_string((reader))); \
            return false;                                                  \
        }                                                                  \
    } while (false)

File_tree* new_File_tree_from_tar(char* path, Read_state* state)
{
    assert(path != NULL);
    assert(state != NULL);
    Read_state_init(state, path);
    struct archive* reader = archive_read_new();
    if (reader == NULL)
    {
        Read_state_set_error(state, "Couldn't allocate memory for the archive reader");
        return NULL;
    }
    int err = ARCHIVE_FATAL;
    err = archive_read_support_compression_bzip2(reader);
    fail_if(err != ARCHIVE_OK, reader, state);
    err = archive_read_support_compression_gzip(reader);
    fail_if(err != ARCHIVE_OK, reader, state);
    err = archive_read_support_format_tar(reader);
    fail_if(err != ARCHIVE_OK, reader, state);

    err = archive_read_open_filename(reader, path, 1024);
    fail_if(err != ARCHIVE_OK, reader, state);

    File_tree* root = NULL;
    struct archive_entry* entry = NULL;
    err = archive_read_next_header(reader, &entry);
    fail_if(err < ARCHIVE_OK, reader, state);
    if (err == ARCHIVE_OK)
    {
        if (archive_entry_filetype(entry) != AE_IFDIR)
        {
            archive_read_finish(reader);
            Read_state_set_error(state, "The archive contains non-directories in the root");
            return NULL;
        }
        const char* path = archive_entry_pathname(entry);
        const char* solidus = strchr(path, '/');
        int len = 0;
        if (solidus == NULL)
        {
            len = strlen(path);
        }
        else
        {
            len = solidus - path;
        }
        char* rname = xcalloc(char, len + 1);
        if (rname == NULL)
        {
            archive_read_finish(reader);
            Read_state_set_error(state, "Couldn't allocate memory for the tree");
            return NULL;
        }
        char* rpath = xcalloc(char, len + 1);
        if (rpath == NULL)
        {
            xfree(rname);
            archive_read_finish(reader);
            Read_state_set_error(state, "Couldn't allocate memory for the tree");
            return NULL;
        }
        strncpy(rname, path, len);
        strncpy(rpath, path, len);
        root = new_File_tree(rname, rpath, NULL);
        if (root == NULL)
        {
            xfree(rpath);
            xfree(rname);
            archive_read_finish(reader);
            Read_state_set_error(state, "Couldn't allocate memory for the tree");
            return NULL;
        }
    }
    char* root_path = File_tree_get_path(root);
    int root_len = strlen(root_path);
    while (err != ARCHIVE_EOF)
    {
        assert(err == ARCHIVE_OK);
        assert(entry != NULL);
        const char* path = archive_entry_pathname(entry);
        const char* solidus = strchr(path, '/');
        if (strncmp(path, root_path, root_len) != 0
                || (path[root_len] != '/' && path[root_len] != '\0'))
        {
            del_File_tree(root);
            archive_read_finish(reader);
            Read_state_set_error(state, "Root of the archive contains several files");
            return NULL;
        }
        if (solidus != NULL)
        {
            while (*solidus == '/')
            {
                ++solidus;
            }
            mode_t type = archive_entry_filetype(entry);
            if (type == AE_IFDIR)
            {
                if (!File_tree_create_branch(root, solidus, NULL))
                {
                    del_File_tree(root);
                    archive_read_finish(reader);
                    Read_state_set_error(state, "Couldn't create path %s", path);
                    return NULL;
                }
            }
            else if (type == AE_IFREG)
            {
                int64_t length = archive_entry_size(entry);
                char* data = xcalloc(char, length + 1);
                if (data == NULL)
                {
                    del_File_tree(root);
                    archive_read_finish(reader);
                    Read_state_set_error(state, "Couldn't allocate memory for %s", path);
                    return NULL;
                }
                long pos = 0;
                char* location = data;
                while (pos < length)
                {
                    ssize_t read = archive_read_data(reader, location, 1024);
                    pos += 1024;
                    location += 1024;
                    if (read < 1024 && pos < length)
                    {
                        xfree(data);
                        del_File_tree(root);
                        archive_read_finish(reader);
                        Read_state_set_error(state, "Couldn't read data from %s", path);
                        return NULL;
                    }
                }
                if (!File_tree_create_branch(root, solidus, data))
                {
                    xfree(data);
                    del_File_tree(root);
                    archive_read_finish(reader);
                    Read_state_set_error(state, "Couldn't load the path %s", path);
                    return NULL;
                }
            }
        }
        err = archive_read_next_header(reader, &entry);
        fail_if(err < ARCHIVE_OK, reader, state);
    }
    archive_read_finish(reader);
    return root;
}

#undef fail_if


int File_tree_cmp(File_tree* tree1, File_tree* tree2)
{
    assert(tree1 != NULL);
    assert(tree2 != NULL);
    int res = strcmp(tree1->name, tree2->name);
    if (res < 0)
    {
        return -1;
    }
    else if (res > 0)
    {
        return 1;
    }
    return 0;
}


char* File_tree_get_name(File_tree* tree)
{
    assert(tree != NULL);
    return tree->name;
}


char* File_tree_get_path(File_tree* tree)
{
    assert(tree != NULL);
    return tree->path;
}


bool File_tree_is_dir(File_tree* tree)
{
    assert(tree != NULL);
    return tree->is_dir;
}


bool File_tree_ins_child(File_tree* tree, File_tree* child)
{
    assert(tree != NULL);
    assert(tree->is_dir);
    assert(child != NULL);
    return AAtree_ins(tree->content.children, child);
}


File_tree* File_tree_get_child(File_tree* tree, char* name)
{
    assert(tree != NULL);
    assert(tree->is_dir);
    assert(name != NULL);
    File_tree* child = &(File_tree){ .is_dir = false, .name = name, .content.data = NULL };
    return AAtree_get_exact(tree->content.children, child);
}


char* File_tree_get_data(File_tree* tree)
{
    assert(tree != NULL);
    assert(!tree->is_dir);
    return tree->content.data;
}


void del_File_tree(File_tree* tree)
{
    assert(tree != NULL);
    if (tree->is_dir)
    {
        assert(tree->content.children != NULL);
        del_AAtree(tree->content.children);
    }
    else
    {
        if (tree->content.data != NULL)
        {
            xfree(tree->content.data);
        }
    }
    xfree(tree->name);
    xfree(tree->path);
    xfree(tree);
    return;
}


