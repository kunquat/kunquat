

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


#ifndef K_FILE_TREE_H
#define K_FILE_TREE_H


#include <stdbool.h>

#include <AAtree.h>


typedef enum
{
    FILE_TREE_DIR,
    FILE_TREE_REG,
    FILE_TREE_SAMPLE
} File_tree_type;


struct Sample;


typedef struct File_tree
{
    File_tree_type type;  ///< Whether the File tree is a directory.
    char* name;           ///< File name.
    char* path;           ///< Path of the file.
    union
    {
        char* data;       ///< Contents of a regular file.
        AAtree* children; ///< Contents of a directory.
        struct Sample* sample;   ///< A Sample.
    } content;
} File_tree;


/**
 * Creates a new File tree.
 *
 * \param name   The (file) name of the tree -- must not be \c NULL. The name
 *               will not be copied, and it must be dynamically allocated.
 * \param path   The (file) path of the tree -- must not be \c NULL. The path
 *               will not be copied, and it must be dynamically allocated.
 * \param data   Contents of a regular file. If this is \c NULL, a directory
 *               structure is created. The data will not be copied, and it
 *               must be dynamically allocated.
 *
 * \return   The new File tree if successful, or \c NULL if memory allocation
 *           failed.
 */
File_tree* new_File_tree(File_tree_type type, char* name, char* path, void* data);


/**
 * Creates a new File tree from a directory tree in a file system.
 *
 * \param path    The (root) path of the directory to be read -- must not be
 *                \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new File tree if successful, otherwise \c NULL.
 */
File_tree* new_File_tree_from_fs(char* path, Read_state* state);


/**
 * Creates a new File tree from a tar archive (optionally with gzip or bzip2
 * compression).
 *
 * \param path    The path of the tarball to be read -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new File tree if successful. On error \c NULL is returned and
 *           errno is set to indicate the error.
 */
File_tree* new_File_tree_from_tar(char* path, Read_state* state);


/**
 * Compares two File tree objects (based on their names).
 *
 * \param tree1   The first File tree -- must not be \c NULL.
 * \param tree2   The second File tree -- must not be \c NULL.
 *
 * \return   -1, 0, or 1 if \a tree1 is found, respectively, to be smaller,
 *           equal to, or greater than \a tree2.
 */
int File_tree_cmp(File_tree* tree1, File_tree* tree2);


/**
 * Tells whether the File tree is a directory.
 *
 * \param tree   The File tree -- must not be \c NULL.
 *
 * \return   \c true if \a tree is a directory, otherwise \c false.
 */
bool File_tree_is_dir(File_tree* tree);


/**
 * Gets the name of the File tree.
 *
 * \param tree   The File tree -- must not be \c NULL.
 *
 * \return   The name -- must not be freed.
 */
char* File_tree_get_name(File_tree* tree);


/**
 * Gets the path of the File tree.
 *
 * \param tree   The File tree -- must not be \c NULL.
 *
 * \return   The path -- must not be freed.
 */
char* File_tree_get_path(File_tree* tree);


/**
 * Inserts a child into the File tree.
 *
 * \param tree    The File tree -- must not be \c NULL and must be a
 *                directory.
 * \param child   The child node -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool File_tree_ins_child(File_tree* tree, File_tree* child);


/**
 * Gets a child from the File tree.
 *
 * \param tree   The File tree -- must not be \c NULL and must be a
 *               directory.
 * \param name   Name of the child.
 *
 * \return   The node matching \a name if found, otherwise \c NULL.
 */
File_tree* File_tree_get_child(File_tree* tree, char* name);


/**
 * Gets the data from the File tree.
 *
 * \param tree   The File tree -- must not be \c NULL and must not be a
 *               directory.
 *
 * \return   The data.
 */
char* File_tree_get_data(File_tree* tree);


/**
 * Gets the Sample from the File tree.
 *
 * \param tree   The File tree -- must not be \c NULL and must contain a
 *               Sample.
 *
 * \return   The Sample.
 */
struct Sample* File_tree_get_sample(File_tree* tree);


/**
 * Removes the Sample from the File tree.
 *
 * \param tree   The File tree -- must not be \c NULL and must contain a
 *               Sample.
 *
 * \return   The Sample.
 */
struct Sample* File_tree_remove_sample(File_tree* tree);


/**
 * Destroys an existing File tree.
 *
 * All the subtrees and file contents will be destroyed.
 *
 * \param tree   The File tree -- must not be \c NULL.
 */
void del_File_tree(File_tree* tree);


#endif // K_FILE_TREE_H


