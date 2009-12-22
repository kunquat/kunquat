

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
#include <string.h>
#include <stddef.h>
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

#include <AAtree.h>
#include <Sample.h>
#include <File_base.h>
#include <File_tree.h>
#include <File_wavpack.h>
#include <Directory.h>
#include <Handle_private.h>

#include <xmemory.h>


File_tree* new_File_tree(File_tree_type type, char* name, char* path, void* data)
{
    assert(type == FILE_TREE_DIR || type == FILE_TREE_REG || type == FILE_TREE_SAMPLE);
    assert(name != NULL);
    assert(path != NULL);
    File_tree* tree = xalloc(File_tree);
    if (tree == NULL)
    {
        return NULL;
    }
    if (type == FILE_TREE_DIR)
    {
        assert(data == NULL);
        AAtree* children = new_AAtree((int (*)(const void*, const void*))File_tree_cmp,
                                      (void (*)(void*))del_File_tree);
        if (children == NULL)
        {
            xfree(tree);
            return NULL;
        }
        tree->content.children = children;
    }
    else if (type == FILE_TREE_SAMPLE)
    {
        assert(data != NULL);
        tree->content.sample = data;
    }
    else
    {
        assert(type == FILE_TREE_REG);
        assert(data != NULL);
        tree->content.data = data;
    }
    tree->type = type;
    tree->name = name;
    tree->path = path;
    return tree;
}


bool has_suffix(const char* str, const char* suffix)
{
    assert(str != NULL);
    assert(suffix != NULL);
    if (suffix[0] == '\0')
    {
        return true;
    }
    int str_end = strlen(str) - 1;
    int suffix_end = strlen(suffix) - 1;
    if (str_end < suffix_end)
    {
        return false;
    }
    for (int i = str_end, j = suffix_end; j >= 0; --i, --j)
    {
        if (str[i] != suffix[j])
        {
            return false;
        }
    }
    return true;
}


File_tree* new_File_tree_from_fs(char* path, kqt_Handle* handle)
{
    assert(path != NULL);
    if (kqt_Handle_get_error(handle)[0] != '\0')
    {
        return NULL;
    }
    Path_type info = path_info(path, handle);
    if (info == PATH_ERROR)
    {
        return NULL;
    }
    int name_start = 0;
    int len = strlen(path);
    char* path_name = xcalloc(char, len + 1);
    if (path_name == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
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
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
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
    if (info == PATH_IS_DIR)
    {
        tree = new_File_tree(FILE_TREE_DIR, name, path_name, NULL);
        if (tree == NULL)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            xfree(name);
            xfree(path_name);
            return NULL;
        }
        Directory* dir = new_Directory(path, handle);
        if (dir == NULL)
        {
            del_File_tree(tree);
            return NULL;
        }
        char* dir_entry = Directory_get_entry(dir);
        while (dir_entry != NULL)
        {
            File_tree* child = new_File_tree_from_fs(dir_entry, handle);
            xfree(dir_entry);
            if (child == NULL)
            {
                del_File_tree(tree);
                del_Directory(dir);
                return NULL;
            }
            if (!File_tree_ins_child(tree, child))
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                del_File_tree(child);
                del_File_tree(tree);
                del_Directory(dir);
                return NULL;
            }
            dir_entry = Directory_get_entry(dir);
        }
        del_Directory(dir);
        if (kqt_Handle_get_error(handle)[0] != '\0')
        {
            del_File_tree(tree);
            return NULL;
        }
    }
    else
    {
        FILE* in = fopen(path, "rb");
        if (in == NULL)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't open file %s for"
                    " reading", __func__, path);
            xfree(name);
            xfree(path_name);
            return NULL;
        }
        if (has_suffix(path, ".wv"))
        {
            Sample* sample = new_Sample();
            if (sample == NULL)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory"
                        " for sample %s", __func__, path);
                xfree(name);
                xfree(path_name);
                return NULL;
            }
            if (!File_wavpack_load_sample(sample, in, NULL, NULL, handle))
            {
                kqt_Handle_set_error(handle, "%s: Couldn't load the WavPack"
                        " file %s", __func__, path);
                del_Sample(sample);
                xfree(name);
                xfree(path_name);
                return NULL;
            }
            tree = new_File_tree(FILE_TREE_SAMPLE, name, path_name, sample);
            if (tree == NULL)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                del_Sample(sample);
                xfree(name);
                xfree(path_name);
                return NULL;
            }
        }
        else
        {
            char* data = read_file(in, handle);
            fclose(in);
            if (data == NULL)
            {
                xfree(name);
                xfree(path_name);
                return NULL;
            }
            assert(data != NULL);
            tree = new_File_tree(FILE_TREE_REG, name, path_name, data);
            if (tree == NULL)
            {
                kqt_Handle_set_error(handle, "Couldn't allocate memory",
                        __func__);
                xfree(data);
                xfree(name);
                xfree(path_name);
                return NULL;
            }
        }
    }
    assert(tree != NULL);
    return tree;
}


bool File_tree_create_branch(File_tree* tree, const char* path, File_tree_type type, void* data)
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
        File_tree* child = new_File_tree(type, cname, cpath, data);
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
        return File_tree_create_branch(child, next_element, type, data);
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
    child = new_File_tree(FILE_TREE_DIR, cname, cpath, NULL);
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
    return File_tree_create_branch(child, next_element, type, data);
}


#define fail_if(cond, reader, handle)                          \
    do                                                         \
    {                                                          \
        if ((cond))                                            \
        {                                                      \
            kqt_Handle_set_error((handle), "%s: %s", __func__, \
                    archive_error_string((reader)));           \
            archive_read_finish((reader));                     \
            return false;                                      \
        }                                                      \
    } while (false)

File_tree* new_File_tree_from_tar(char* path, kqt_Handle* handle)
{
    assert(path != NULL);
    if (kqt_Handle_get_error(handle)[0] != '\0')
    {
        return NULL;
    }
    struct archive* reader = archive_read_new();
    if (reader == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory for the"
                " archive reader", __func__);
        return NULL;
    }
    int err = ARCHIVE_FATAL;
    err = archive_read_support_compression_bzip2(reader);
    fail_if(err != ARCHIVE_OK, reader, handle);
    err = archive_read_support_compression_gzip(reader);
    fail_if(err != ARCHIVE_OK, reader, handle);
    err = archive_read_support_format_tar(reader);
    fail_if(err != ARCHIVE_OK, reader, handle);

    err = archive_read_open_filename(reader, path, 1024);
    fail_if(err != ARCHIVE_OK, reader, handle);

    File_tree* root = NULL;
    struct archive_entry* entry = NULL;
    err = archive_read_next_header(reader, &entry);
    fail_if(err < ARCHIVE_OK, reader, handle);
    if (err == ARCHIVE_OK)
    {
        if (archive_entry_filetype(entry) != AE_IFDIR)
        {
            archive_read_finish(reader);
            kqt_Handle_set_error(handle, "%s: The archive %s contains"
                    " non-directories in the root", __func__, path);
            return NULL;
        }
        const char* entry_path = archive_entry_pathname(entry);
        const char* solidus = strchr(entry_path, '/');
        int len = 0;
        if (solidus == NULL)
        {
            len = strlen(entry_path);
        }
        else
        {
            len = solidus - entry_path;
        }
        char* rname = xcalloc(char, len + 1);
        if (rname == NULL)
        {
            archive_read_finish(reader);
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            return NULL;
        }
        char* rpath = xcalloc(char, len + 1);
        if (rpath == NULL)
        {
            xfree(rname);
            archive_read_finish(reader);
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            return NULL;
        }
        strncpy(rname, entry_path, len);
        strncpy(rpath, entry_path, len);
        root = new_File_tree(FILE_TREE_DIR, rname, rpath, NULL);
        if (root == NULL)
        {
            xfree(rpath);
            xfree(rname);
            archive_read_finish(reader);
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            return NULL;
        }
    }
    char* root_path = File_tree_get_path(root);
    int root_len = strlen(root_path);
    while (err != ARCHIVE_EOF)
    {
        assert(err == ARCHIVE_OK);
        assert(entry != NULL);
        const char* entry_path = archive_entry_pathname(entry);
        const char* solidus = strchr(entry_path, '/');
        if (strncmp(entry_path, root_path, root_len) != 0
                || (entry_path[root_len] != '/' && entry_path[root_len] != '\0'))
        {
            del_File_tree(root);
            archive_read_finish(reader);
            kqt_Handle_set_error(handle, "%s: Root of the archive %s"
                    " contains several files", __func__, path);
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
                if (!File_tree_create_branch(root, solidus, FILE_TREE_DIR, NULL))
                {
                    del_File_tree(root);
                    archive_read_finish(reader);
                    kqt_Handle_set_error(handle, "%s: Couldn't create path %s",
                            __func__, entry_path);
                    return NULL;
                }
            }
            else if (type == AE_IFREG)
            {
                if (has_suffix(solidus, ".wv"))
                {
                    Sample* sample = new_Sample();
                    if (sample == NULL)
                    {
                        del_File_tree(root);
                        archive_read_finish(reader);
                        kqt_Handle_set_error(handle, "%s: Couldn't allocate"
                                " memory for sample %s", __func__, entry_path);
                        return NULL;
                    }
                    if (!File_wavpack_load_sample(sample, NULL, reader, entry, handle))
                    {
                        del_Sample(sample);
                        del_File_tree(root);
                        archive_read_finish(reader);
                        kqt_Handle_set_error(handle, "%s: Couldn't load the"
                                " WavPack file %s", __func__, entry_path);
                        return NULL;
                    }
                    if (!File_tree_create_branch(root, solidus, FILE_TREE_SAMPLE, sample))
                    {
                        del_Sample(sample);
                        del_File_tree(root);
                        archive_read_finish(reader);
                        kqt_Handle_set_error(handle, "%s: Couldn't load the"
                                " path %s", __func__, entry_path);
                        return NULL;
                    }
                }
                else
                {
                    int64_t length = archive_entry_size(entry);
                    char* data = xcalloc(char, length + 1);
                    if (data == NULL)
                    {
                        del_File_tree(root);
                        archive_read_finish(reader);
                        kqt_Handle_set_error(handle, "%s: Couldn't allocate"
                                " memory for %s", __func__, entry_path);
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
                            kqt_Handle_set_error(handle, "%s: Couldn't read"
                                    " data from %s", __func__, entry_path);
                            return NULL;
                        }
                    }
                    if (!File_tree_create_branch(root, solidus, FILE_TREE_REG, data))
                    {
                        xfree(data);
                        del_File_tree(root);
                        archive_read_finish(reader);
                        kqt_Handle_set_error(handle, "%s: Couldn't load the"
                                " path %s", __func__, entry_path);
                        return NULL;
                    }
                }
            }
        }
        err = archive_read_next_header(reader, &entry);
        fail_if(err < ARCHIVE_OK, reader, handle);
    }
    archive_read_finish(reader);
    return root;
}

#undef fail_if


int File_tree_cmp(const File_tree* tree1, const File_tree* tree2)
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
    return tree->type == FILE_TREE_DIR;
}


bool File_tree_ins_child(File_tree* tree, File_tree* child)
{
    assert(tree != NULL);
    assert(tree->type == FILE_TREE_DIR);
    assert(child != NULL);
    return AAtree_ins(tree->content.children, child);
}


File_tree* File_tree_get_child(File_tree* tree, char* name)
{
    assert(tree != NULL);
    assert(tree->type == FILE_TREE_DIR);
    assert(name != NULL);
    File_tree* child = &(File_tree){ .type = FILE_TREE_REG, .name = name, .content.data = NULL };
    return AAtree_get_exact(tree->content.children, child);
}


char* File_tree_get_data(File_tree* tree)
{
    assert(tree != NULL);
    assert(tree->type == FILE_TREE_REG);
    return tree->content.data;
}


struct Sample* File_tree_get_sample(File_tree* tree)
{
    assert(tree != NULL);
    assert(tree->type == FILE_TREE_SAMPLE);
    return tree->content.sample;
}


struct Sample* File_tree_remove_sample(File_tree* tree)
{
    assert(tree != NULL);
    assert(tree->type == FILE_TREE_SAMPLE);
    Sample* sample = tree->content.sample;
    tree->content.sample = NULL;
    return sample;
}


void del_File_tree(File_tree* tree)
{
    assert(tree != NULL);
    if (tree->type == FILE_TREE_DIR)
    {
        assert(tree->content.children != NULL);
        del_AAtree(tree->content.children);
    }
    else if (tree->type == FILE_TREE_REG)
    {
        if (tree->content.data != NULL)
        {
            xfree(tree->content.data);
        }
    }
    else
    {
        assert(tree->type == FILE_TREE_SAMPLE);
        if (tree->content.sample != NULL)
        {
            del_Sample(tree->content.sample);
        }
    }
    xfree(tree->name);
    xfree(tree->path);
    xfree(tree);
    return;
}


