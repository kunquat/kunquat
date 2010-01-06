

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <archive.h>
#include <archive_entry.h>

#include <File_kqt.h>
#include <Handle_r.h>
#include <Entries.h>
#include <Parse_manager.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <math_common.h>

#include <xmemory.h>


#define fail_if(cond, reader, handle)                          \
    if (true)                                                  \
    {                                                          \
        if ((cond))                                            \
        {                                                      \
            kqt_Handle_set_error((handle), "%s: %s", __func__, \
                    archive_error_string((reader)));           \
            archive_read_finish((reader));                     \
            return false;                                      \
        }                                                      \
    } else (void)0

bool File_kqt_open(Handle_r* handle_r, const char* path)
{
    assert(handle_is_valid((kqt_Handle*)handle_r));
    assert(handle_r->handle.mode == KQT_READ);
    assert(path != NULL);
    struct archive* reader = archive_read_new();
    if (reader == NULL)
    {
        kqt_Handle_set_error(&handle_r->handle,
                "%s: Couldn't allocate memory", __func__);
        return false;
    }
    int err = ARCHIVE_FATAL;
    err = archive_read_support_compression_bzip2(reader);
    fail_if(err != ARCHIVE_OK, reader, &handle_r->handle);
    err = archive_read_support_compression_gzip(reader);
    fail_if(err != ARCHIVE_OK, reader, &handle_r->handle);
    err = archive_read_support_format_tar(reader);
    fail_if(err != ARCHIVE_OK, reader, &handle_r->handle);

    err = archive_read_open_filename(reader, path, 1024);
    fail_if(err != ARCHIVE_OK, reader, &handle_r->handle);

    struct archive_entry* entry = NULL;
    err = archive_read_next_header(reader, &entry);
    while (err == ARCHIVE_OK)
    {
        assert(entry != NULL);
        if (archive_format(reader) != ARCHIVE_FORMAT_TAR_USTAR)
        {
            kqt_Handle_set_error(&handle_r->handle, "%s: The file %s has an"
                    " incorrect archive format (should be ustar)", __func__, path);
            archive_read_finish(reader);
            return false;
        }
        const char* entry_path = archive_entry_pathname(entry);
        const char* header = "kunquatc" KQT_FORMAT_VERSION;
        if (!string_has_prefix(entry_path, header) ||
                (entry_path[strlen(header)] != '/' &&
                entry_path[strlen(header)] != '\0'))
        {
            const char* other_header = "kunquati";
            if (parse_index_dir(entry_path, other_header, 2) >= 0)
            {
                kqt_Handle_set_error(&handle_r->handle, "%s: Cannot open"
                        " Kunquat instruments as compositions", __func__);
            }
            else if (other_header = "kunquats",
                    parse_index_dir(entry_path, other_header, 2) >= 0)
            {
                kqt_Handle_set_error(&handle_r->handle, "%s: Cannot open"
                        " Kunquat scales as compositions", __func__);
            }
            else
            {
                kqt_Handle_set_error(&handle_r->handle, "%s: The .kqt file %s contains"
                        " an invalid data entry: %s", __func__, path, entry_path);
            }
            archive_read_finish(reader);
            return false;
        }
        const char* key = strchr(entry_path, '/');
        if (key != NULL && archive_entry_filetype(entry) == AE_IFREG)
        {
            ++key;
            int64_t length = archive_entry_size(entry);
            char* data = xnalloc(char, length);
            if (data == NULL)
            {
                kqt_Handle_set_error(&handle_r->handle,
                        "%s: Couldn't allocate memory", __func__);
                archive_read_finish(reader);
                return false;
            }
            long pos = 0;
            char* location = data;
            while (pos < length)
            {
                ssize_t read = archive_read_data(reader, location,
                                                 MIN(1024, length - pos));
                pos += read;
                location += read;
                if (read < 1024 && pos < length)
                {
                    kqt_Handle_set_error(&handle_r->handle, ":%s: Couldn't read"
                            " data from %s:%s", __func__, path, entry_path);
                    xfree(data);
                    return false;
                }
            }
            if (!parse_data(&handle_r->handle, key, data, length))
            {
                archive_read_finish(reader);
                xfree(data);
                return false;
            }
            if (!Entries_set(handle_r->entries, key, data, length))
            {
                kqt_Handle_set_error(&handle_r->handle,
                        "%s: Couldn't allocate memory", __func__);
                archive_read_finish(reader);
                xfree(data);
                return false;
            }
        }
        err = archive_read_next_header(reader, &entry);
    }
    if (err < ARCHIVE_OK)
    {
        kqt_Handle_set_error(&handle_r->handle, "%s: Couldn't open the file %s"
                " as a Kunquat file: %s", __func__, path,
                archive_error_string(reader));
        archive_read_finish(reader);
        return false;
    }
    archive_read_finish(reader);
    return true;
}

#undef fail_if


