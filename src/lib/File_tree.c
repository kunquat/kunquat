

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

#include <AAtree.h>

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


char* File_tree_get_data(File_tree* tree);
{
    assert(tree != NULL);
    assert(!tree->is_dir);
    return tree->content.data;
}


void del_File_tree(File_tree* tree)
{
    assert(tree != NULL);
    if (is_dir)
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


