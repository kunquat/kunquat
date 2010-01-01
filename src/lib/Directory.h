

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


#ifndef K_DIRECTORY_H
#define K_DIRECTORY_H


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <Handle_private.h>


/**
 * Directory is a wrapper for reading directories.
 *
 * This structure is intended to be used only in special cases where the
 * helper functions below don't provide the required functionality.
 */
typedef struct Directory Directory;


/**
 * Identifier for different path types.
 */
typedef enum
{
    PATH_ERROR = -1,
    PATH_NO_ENTRY,
    PATH_IS_REGULAR,
    PATH_IS_DIR,
    PATH_IS_OTHER
} Path_type;


/**
 * Resolves the absolute path of the given path.
 *
 * \param path     The path -- must not be \c NULL.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   The new path if successful, or \c NULL if memory allocation
 *           failed.
 */
char* absolute_path(const char* path, kqt_Handle* handle);


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
char* append_to_path(const char* path, const char* name);


/**
 * Gets the last element of a path.
 *
 * \param path   The path -- must not be \c NULL.
 *
 * \return   The last element.
 */
char* last_element(char* path);


/**
 * Creates a directory in the file system.
 *
 * If the new directory is inside a directory accessed via a Directory
 * object, the Directory should be closed and created anew.
 *
 * \param path     The path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool create_dir(const char* path, kqt_Handle* handle);


/**
 * Tells information about the given path.
 *
 * \param path     The path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   The path info.
 */
Path_type path_info(const char* path, kqt_Handle* handle);


/**
 * Tells information about the given file.
 *
 * \param file     The file -- must not be \c NULL.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   The path info.
 */
Path_type file_info(FILE* file, kqt_Handle* handle);


/**
 * Tells the file size of the given path.
 *
 * \param path     The path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   The size of the file if one exists, 0 if it does not, or -1 if
 *           an error occurred while inspecting the path.
 */
long path_size(const char* path, kqt_Handle* handle);


/**
 * Tells the size of the given file.
 *
 * \param file     The file -- must not be \c NULL.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   The size of the file if one exists, or -1 if an error occurred
 *           while inspecting the path.
 */
long file_size(FILE* file, kqt_Handle* handle);


/**
 * Copies a directory tree.
 *
 * \param dest     The destination path -- must not be \c NULL or an empty
 *                 string.
 * \param src      The source path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   \c true if successful, otherwise \c false. The path is most
 *           likely only partially created in case of an error.
 */
bool copy_dir(const char* dest, const char* src, kqt_Handle* handle);


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
bool copy_file(const char* dest, const char* src, kqt_Handle* handle);


/**
 * Moves a directory tree.
 *
 * \param dest     The destination path -- must not be \c NULL or an empty
 *                 string.
 * \param src      The source path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool move_dir(const char* dest, const char* src, kqt_Handle* handle);


/**
 * Removes a directory tree.
 *
 * \param path     The path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   \c true if successful, otherwise \c false. The path is most
 *           likely partially destroyed in case of an error.
 */
bool remove_dir(const char* path, kqt_Handle* handle);


/**
 * Opens an existing directory.
 *
 * \param path     The path -- must not be \c NULL or an empty string.
 * \param handle   The Kunquat Handle associated with the path, or \c NULL if
 *                 one does not exist. This is used for error reporting.
 *
 * \return   The Directory if successful, otherwise \c NULL.
 */
Directory* new_Directory(const char* path, kqt_Handle* handle);


/**
 * Gets a Directory entry.
 *
 * \param dir   The Directory -- must not be \c NULL.
 *
 * \return   The path of the entry if one exists, otherwise \c NULL. The
 *           caller must check the Kunquat Handle (or \c NULL) supplied with
 *           new_Directory for errors. Also, the caller must eventually free
 *           the returned string with xfree.
 */
char* Directory_get_entry(Directory* dir);


/**
 * Destroys an existing Directory structure.
 *
 * The file system path associated with the Directory will not be removed.
 *
 * \param dir   The Directory -- must not be \c NULL.
 */
void del_Directory(Directory* dir);


#endif // K_DIRECTORY_H


