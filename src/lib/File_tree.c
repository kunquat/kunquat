

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

#include <AAtree.h>
#include <File_base.h>
#include <File_tree.h>

#include <xmemory.h>


File_tree* new_File_tree(char* name, char* data)
{
    assert(name != NULL);
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
        tree = new_File_tree(name, NULL);
        if (tree == NULL)
        {
            xfree(name);
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
            return NULL;
        }
        char* data = read_file(in, state);
        fclose(in);
        if (state->error)
        {
            xfree(name);
            return NULL;
        }
        assert(data != NULL);
        tree = new_File_tree(name, data);
        if (tree == NULL)
        {
            xfree(data);
            xfree(name);
            return NULL;
        }
    }
    assert(tree != NULL);
    return tree;
}


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
        assert(tree->content.data != NULL);
        xfree(tree->content.data);
    }
    xfree(tree->name);
    xfree(tree);
    return;
}


