

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
#include <stdbool.h>
#include <string.h>

#include <archive.h>
#include <archive_entry.h>

#include <File_base.h>
#include <File_kqt.h>
#include <Handle_r.h>
#include <Entries.h>
#include <Parse_manager.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


#define fail_if(cond, reader, handle)                            \
    if (true)                                                    \
    {                                                            \
        if ((cond))                                              \
        {                                                        \
            kqt_Handle_set_error((handle), ERROR_RESOURCE, "%s", \
                    archive_error_string((reader)));             \
            archive_read_finish((reader));                       \
            return false;                                        \
        }                                                        \
    } else (void)0

bool File_kqt_open(Handle_r* handle_r, const char* path)
{
    assert(handle_is_valid((kqt_Handle*)handle_r));
    assert(handle_r->handle.mode == KQT_READ);
    assert(path != NULL);
    struct archive* reader = archive_read_new();
    if (reader == NULL)
    {
        kqt_Handle_set_error(&handle_r->handle, ERROR_MEMORY,
                "Couldn't allocate memory");
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
            kqt_Handle_set_error(&handle_r->handle, ERROR_FORMAT, "The file %s"
                    " has an incorrect archive format (should be ustar)", path);
            archive_read_finish(reader);
            return false;
        }
        const char* entry_path = archive_entry_pathname(entry);
        const char* header = MAGIC_ID "c" KQT_FORMAT_VERSION;
        if (!string_has_prefix(entry_path, header) ||
                (entry_path[strlen(header)] != '/' &&
                entry_path[strlen(header)] != '\0'))
        {
            const char* other_header = MAGIC_ID "i";
            const char* file_type = NULL;
            if (string_extract_index(entry_path, other_header, 2) >= 0)
            {
                file_type = "instrument";
            }
            else if (other_header = MAGIC_ID "g",
                    string_extract_index(entry_path, other_header, 2) >= 0)
            {
                file_type = "generator";
            }
            else if (other_header = MAGIC_ID "e",
                    string_extract_index(entry_path, other_header, 2) >= 0)
            {
                file_type = "DSP effect";
            }
            else if (other_header = MAGIC_ID "s",
                    string_extract_index(entry_path, other_header, 2) >= 0)
            {
                file_type = "scale";
            }
            if (file_type != NULL)
            {
                kqt_Handle_set_error(&handle_r->handle, ERROR_FORMAT,
                        "The file %s appears to be a Kunquat %s,"
                        " not composition", path, file_type);
            }
            else
            {
                kqt_Handle_set_error(&handle_r->handle, ERROR_FORMAT,
                        "The file %s contains an invalid data entry: %s",
                        path, entry_path);
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
                kqt_Handle_set_error(&handle_r->handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
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
                    kqt_Handle_set_error(&handle_r->handle, ERROR_RESOURCE,
                            "Couldn't read data from file %s, entry %s",
                            path, entry_path);
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
                kqt_Handle_set_error(&handle_r->handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                archive_read_finish(reader);
                xfree(data);
                return false;
            }
        }
        err = archive_read_next_header(reader, &entry);
    }
    if (err < ARCHIVE_OK)
    {
        kqt_Handle_set_error(&handle_r->handle, ERROR_RESOURCE,
                "Couldn't open the file %s as a Kunquat file: %s",
                path, archive_error_string(reader));
        archive_read_finish(reader);
        return false;
    }
    archive_read_finish(reader);
    return true;
}

#undef fail_if


