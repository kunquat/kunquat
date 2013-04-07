

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#include <Handle_private.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <Song.h>
#include <Playdata.h>
#include <Voice_pool.h>
#include <xassert.h>


static kqt_Handle* handles[KQT_HANDLES_MAX] = { NULL };

// For errors without an associated Kunquat Handle.
static char null_error[KQT_HANDLE_ERROR_LENGTH] = { '\0' };


static bool add_handle(kqt_Handle* handle);

static bool remove_handle(kqt_Handle* handle);


static int ptrcmp(const void* ptr1, const void* ptr2);


bool kqt_Handle_init(kqt_Handle* handle, long buffer_size)
{
    assert(handle != NULL);
    assert(buffer_size > 0);
    if (!add_handle(handle))
    {
        return false;
    }
    handle->data_is_valid = true;
    handle->data_is_validated = true;
    handle->song = NULL;
    handle->destroy = NULL;
    handle->get_data = NULL;
    handle->get_data_length = NULL;
    handle->set_data = NULL;
    handle->error[0] = handle->error[KQT_HANDLE_ERROR_LENGTH - 1] = '\0';
    handle->position[0] = handle->position[POSITION_LENGTH - 1] = '\0';

//    int buffer_count = SONG_DEFAULT_BUF_COUNT;
//    int voice_count = 256;

    handle->returned_values = new_AAtree(ptrcmp, memory_free);
    if (handle->returned_values == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        bool removed = remove_handle(handle);
        assert(removed);
        (void)removed;
        return false;
    }
    handle->song = new_Song(buffer_size);
    if (handle->song == NULL)
    {
        kqt_Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        bool removed = remove_handle(handle);
        assert(removed);
        (void)removed;
        del_AAtree(handle->returned_values);
        return false;
    }

    kqt_Handle_stop(handle);
    kqt_Handle_set_position(handle, 0, 0);
    return true;
}


char* kqt_Handle_get_error(kqt_Handle* handle)
{
    if (!handle_is_valid(handle))
    {
        return null_error;
    }
    return handle->error;
}


void kqt_Handle_clear_error(kqt_Handle* handle)
{
    check_data_is_valid_void(handle);
    if (!handle_is_valid(handle))
    {
        null_error[0] = '\0';
        return;
    }
    handle->error[0] = '\0';
    return;
}


#define set_invalid_if(cond, ...)                                      \
    if (true)                                                          \
    {                                                                  \
        if (cond)                                                      \
        {                                                              \
            kqt_Handle_set_error((handle), ERROR_FORMAT, __VA_ARGS__); \
            (handle)->data_is_valid = false;                           \
            return 0;                                                  \
        }                                                              \
    } else (void)0

int kqt_Handle_validate(kqt_Handle* handle)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);

    // Check album
    if (handle->song->album_is_existent)
    {
        const Track_list* tl = handle->song->track_list;
        set_invalid_if(
                tl == NULL,
                "Album does not contain a track list");
        set_invalid_if(
                Track_list_get_len(tl) == 0,
                "Album has no tracks");
    }

    // Check songs
    for (int i = 0; i < KQT_SONGS_MAX; ++i)
    {
        if (!Subsong_table_get_existent(handle->song->subsongs, i))
            continue;

        // Check for orphans
        const Track_list* tl = handle->song->track_list;
        set_invalid_if(
                !handle->song->album_is_existent || tl == NULL,
                "Module contains song %d but no album", i);

        bool found = false;
        for (size_t k = 0; k < Track_list_get_len(tl); ++k)
        {
            if (Track_list_get_song_index(tl, k) == i)
            {
                found = true;
                break;
            }
        }
        set_invalid_if(!found, "Song %d is not included in the album", i);

        // Check for empty songs
        const Order_list* ol = handle->song->order_lists[i];
        set_invalid_if(
                ol == NULL || Order_list_get_len(ol) == 0,
                "Song %d does not contain systems", i);
    }

    // Check for nonexistent songs in the track list
    if (handle->song->album_is_existent)
    {
        const Track_list* tl = handle->song->track_list;
        assert(tl != NULL);

        for (size_t i = 0; i < Track_list_get_len(tl); ++i)
        {
            set_invalid_if(
                    !Subsong_table_get_existent(handle->song->subsongs,
                        Track_list_get_song_index(tl, i)),
                    "Album includes nonexistent song %d", i);
        }
    }

    // Check existing patterns
    for (int i = 0; i < KQT_PATTERNS_MAX; ++i)
    {
        if (!Pat_table_get_existent(handle->song->pats, i))
            continue;

        Pattern* pat = Pat_table_get(handle->song->pats, i);
        set_invalid_if(
                pat == NULL,
                "Pattern %d exists but contains no data", i);

        bool pattern_has_instance = false;
        for (int k = 0; k < KQT_PAT_INSTANCES_MAX; ++k)
        {
            if (Pattern_get_inst_existent(pat, k))
            {
                // Mark found instance
                pattern_has_instance = true;

                // Check that the instance is used in the album
                set_invalid_if(
                        !handle->song->album_is_existent,
                        "Pattern instance [%d, %d] exists but no album"
                        " is present", i, k);

                bool instance_found = false;

                const Track_list* tl = handle->song->track_list;
                assert(tl != NULL);

                for (size_t track = 0; track < Track_list_get_len(tl); ++track)
                {
                    const int song_index = Track_list_get_song_index(tl, track);

                    if (!Subsong_table_get_existent(
                                handle->song->subsongs,
                                song_index))
                        continue;

                    const Order_list* ol = handle->song->order_lists[song_index];
                    assert(ol != NULL);

                    for (size_t system = 0; system < Order_list_get_len(ol); ++system)
                    {
                        const Pat_inst_ref* piref = Order_list_get_pat_inst_ref(
                                ol, system);
                        if (piref->pat == i && piref->inst == k)
                        {
                            set_invalid_if(
                                    instance_found,
                                    "Duplicate occurrence of pattern instance"
                                    " [%d, %d]", i, k);
                            instance_found = true;
                        }
                    }
                }

                set_invalid_if(
                        !instance_found,
                        "Pattern instance [%d, %d] exists but is not used",
                        i, k);
            }
        }

        set_invalid_if(
                !pattern_has_instance,
                "Pattern %d exists but has no instances", i);
    }

    handle->data_is_validated = true;
    return 1;
}

#undef set_invalid_if


void kqt_Handle_set_error_(kqt_Handle* handle,
                           Error_type type,
                           const char* file,
                           int line,
                           const char* func,
                           const char* message, ...)
{
    assert(type > ERROR_NONE);
    assert(type < ERROR_LAST);
    assert(file != NULL);
    assert(line >= 0);
    assert(func != NULL);
    assert(message != NULL);
    char err_str[KQT_HANDLE_ERROR_LENGTH] = { '\0' };
    static const char* error_codes[ERROR_LAST] =
    {
        [ERROR_ARGUMENT] = "ArgumentError",
        [ERROR_FORMAT] = "FormatError",
        [ERROR_MEMORY] = "MemoryError",
        [ERROR_RESOURCE] = "ResourceError",
    };
    strcpy(err_str, "{ \"type\": \"");
    strcat(err_str, error_codes[type]);
    strcat(err_str, "\", ");

    strcat(err_str, "\"file\": \"");
    strcat(err_str, file);
    strcat(err_str, "\", ");

    sprintf(&err_str[strlen(err_str)], "\"line\": %d, ", line);

    strcat(err_str, "\"function\": \"");
    strcat(err_str, func);
    strcat(err_str, "\", ");

    strcat(err_str, "\"message\": \"");
    char message_str[KQT_HANDLE_ERROR_LENGTH] = { '\0' };
    va_list args;
    va_start(args, message);
    vsnprintf(message_str, KQT_HANDLE_ERROR_LENGTH, message, args);
    va_end(args);
    int json_pos = strlen(err_str);
    for (int i = 0; json_pos < KQT_HANDLE_ERROR_LENGTH - 4 &&
                    message_str[i] != '\0'; ++i, ++json_pos)
    {
        char ch = message_str[i];
        static const char named_controls[] = "\"\\\b\f\n\r\t";
        static const char* named_replace[] =
                { "\\\"", "\\\\", "\\b", "\\f", "\\n", "\\r", "\\t" };
        const char* named_control = strchr(named_controls, ch);
        if (named_control != NULL)
        {
            if (json_pos >= KQT_HANDLE_ERROR_LENGTH - 5)
            {
                break;
            }
            int pos = named_control - named_controls;
            assert(pos >= 0);
            assert(pos < (int)strlen(named_controls));
            strcpy(&err_str[json_pos], named_replace[pos]);
            json_pos += strlen(named_replace[pos]) - 1;
        }
        else if (ch < 0x20 || ch == 0x7f)
        {
            if (json_pos >= KQT_HANDLE_ERROR_LENGTH - 4 - 5)
            {
                break;
            }
            // FIXME: We should really check for all control characters
            char code[] = "\\u0000";
            snprintf(code, strlen(code) + 1, "\\u%04x", ch);
            strcpy(&err_str[json_pos], code);
            json_pos += strlen(code) - 1;
        }
        else
        {
            err_str[json_pos] = ch;
        }
    }
    strcat(err_str, "\" }");
    err_str[KQT_HANDLE_ERROR_LENGTH - 1] = '\0';

    strcpy(null_error, err_str);
    if (handle != NULL)
    {
        assert(handle_is_valid(handle));
        strcpy(handle->error, err_str);
    }
    return;
}


#if 0
void* kqt_Handle_get_data(kqt_Handle* handle, const char* key)
{
    check_handle(handle, NULL);
    check_key(handle, key, NULL);
    if (handle->mode == KQT_MEM)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Cannot get data from a write-only Kunquat Handle.");
        return NULL;
    }
    assert(handle->get_data != NULL);
    void* data = handle->get_data(handle, key);
    if (data != NULL)
    {
        assert(AAtree_get_exact(handle->returned_values, data) == NULL);
        if (!AAtree_ins(handle->returned_values, data))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            memory_free(data);
            return NULL;
        }
    }
    return data;
}


long kqt_Handle_get_data_length(kqt_Handle* handle, const char* key)
{
    check_handle(handle, -1);
    check_key(handle, key, -1);
    if (handle->mode == KQT_MEM)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Cannot get data from a write-only Kunquat Handle.");
        return -1;
    }
    assert(handle->get_data_length != NULL);
    return handle->get_data_length(handle, key);
}
#endif


int kqt_Handle_set_data(kqt_Handle* handle,
                        const char* key,
                        void* data,
                        long length)
{
    check_handle(handle, 0);
    check_data_is_valid(handle, 0);
    check_key(handle, key, 0);
#if 0
    if (handle->mode == KQT_READ)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Cannot set data on a read-only Kunquat Handle.");
        return 0;
    }
#endif
    assert(handle->set_data != NULL);
    return handle->set_data(handle, key, data, length);
}


int kqt_Handle_free_data(kqt_Handle* handle, void* data)
{
    check_handle(handle, 0);
    if (data == NULL)
    {
        return 1;
    }
    void* target = NULL;
    if ((target = AAtree_remove(handle->returned_values, data)) == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT,
                "Data %p does not originate from this Handle", data);
        return 0;
    }
    memory_free(target);
    return 1;
}


bool key_is_valid(kqt_Handle* handle, const char* key)
{
    assert(handle_is_valid(handle));
    if (key == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "No key given");
        return false;
    }
    if (strlen(key) > KQT_KEY_LENGTH_MAX)
    {
        char key_repr[KQT_KEY_LENGTH_MAX + 3] = { '\0' };
        strncpy(key_repr, key, KQT_KEY_LENGTH_MAX - 1);
        strcat(key_repr, "...");
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s is too long"
                " (over %d characters)", key_repr, KQT_KEY_LENGTH_MAX);
        return false;
    }
    bool valid_element = false;
    bool element_has_period = false;
    const char* key_iter = key;
    while (*key_iter != '\0')
    {
        if (!(*key_iter >= '0' && *key_iter <= '9') &&
                strchr("abcdefghijklmnopqrstuvwxyz_./X", *key_iter) == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains an"
                    " illegal character \'%c\'", key, *key_iter);
            return false;
        }
        if (*key_iter != '.' && *key_iter != '/')
        {
            valid_element = true;
        }
        else if (*key_iter == '.')
        {
            element_has_period = true;
        }
        else if (*key_iter == '/')
        {
            if (!valid_element)
            {
                kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains"
                        " an invalid component", key);
                return false;
            }
            else if (element_has_period)
            {
                kqt_Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains"
                        " an intermediate component with a period", key);
                return false;
            }
            valid_element = false;
            element_has_period = false;
        }
        ++key_iter;
    }
    if (!element_has_period)
    {
        kqt_Handle_set_error(handle, ERROR_ARGUMENT, "The final element of"
                " key %s does not have a period", key);
        return false;
    }
    return true;
}


void kqt_del_Handle(kqt_Handle* handle)
{
    check_handle_void(handle);
    if (!remove_handle(handle))
    {
        kqt_Handle_set_error(NULL, ERROR_ARGUMENT,
                "Invalid Kunquat Handle: %p", (void*)handle);
        return;
    }
    if (handle->song != NULL)
    {
        del_Song(handle->song);
        handle->song = NULL;
    }
    if (handle->returned_values != NULL)
    {
        del_AAtree(handle->returned_values);
        handle->returned_values = NULL;
    }
    assert(handle->destroy != NULL);
    handle->destroy(handle);
    return;
}


static int ptrcmp(const void* ptr1, const void* ptr2)
{
    if (ptr1 < ptr2)
    {
        return -1;
    }
    else if (ptr1 > ptr2)
    {
        return 1;
    }
    return 0;
}


static bool add_handle(kqt_Handle* handle)
{
    assert(handle != NULL);
#ifndef NDEBUG
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        assert(handles[i] != handle);
    }
#endif
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        if (handles[i] == NULL)
        {
            handles[i] = handle;
            return true;
        }
    }
    kqt_Handle_set_error(NULL, ERROR_MEMORY,
            "Maximum number of Kunquat Handles reached");
    return false;
}


bool handle_is_valid(kqt_Handle* handle)
{
    if (handle == NULL)
    {
        return false;
    }
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        if (handles[i] == handle)
        {
#ifndef NDEBUG
            for (int k = i + 1; k < KQT_HANDLES_MAX; ++k)
            {
                assert(handles[k] != handle);
            }
#endif
            return true;
        }
    }
    return false;
}


static bool remove_handle(kqt_Handle* handle)
{
    assert(handle != NULL);
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        if (handles[i] == handle)
        {
            handles[i] = NULL;
#ifndef NDEBUG
            for (int k = i + 1; k < KQT_HANDLES_MAX; ++k)
            {
                assert(handles[k] != handle);
            }
#endif
            return true;
        }
    }
    return false;
}


