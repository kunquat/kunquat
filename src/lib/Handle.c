

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <debug/assert.h>
#include <Handle_private.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <module/Module.h>
#include <module/Parse_manager.h>
#include <string/common.h>


static Handle* handles[KQT_HANDLES_MAX] = { NULL };

// For errors without an associated Kunquat Handle.
static Error null_error = { "", ERROR_COUNT_ };


static bool remove_handle(kqt_Handle handle);


void Handle_deinit(Handle* handle);


static kqt_Handle add_handle(Handle* handle)
{
    assert(handle != NULL);

#ifndef NDEBUG
    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
        assert(handles[i] != handle);
#endif

    static int next_try = 0;

    for (int i = 0; i < KQT_HANDLES_MAX; ++i)
    {
        const int try = (i + next_try) % KQT_HANDLES_MAX;
        if (handles[try] == NULL)
        {
            handles[try] = handle;
            next_try = try + 1;
            return try + 1; // shift kqt_Handle range to [1, KQT_HANDLES_MAX]
        }
    }

    Handle_set_error(NULL, ERROR_MEMORY,
            "Maximum number of Kunquat Handles reached");
    return 0;
}


Handle* get_handle(kqt_Handle id)
{
    assert(kqt_Handle_is_valid(id));

    Handle* handle = handles[id - 1];
    assert(handle != NULL);

    return handle;
}


static bool remove_handle(kqt_Handle id)
{
    assert(kqt_Handle_is_valid(id));

    const bool was_null = (handles[id - 1] == NULL);
    handles[id - 1] = NULL;

    return !was_null;
}


kqt_Handle kqt_new_Handle(void)
{
    Handle* handle = memory_alloc_item(Handle);
    if (handle == NULL)
    {
        Handle_set_error(0, ERROR_MEMORY, "Couldn't allocate memory");
        return 0;
    }

    if (!Handle_init(handle, DEFAULT_BUFFER_SIZE))
    {
        memory_free(handle);
        return 0;
    }

    kqt_Handle id = add_handle(handle);
    if (id == 0)
    {
        Handle_deinit(handle);
        memory_free(handle);
        return 0;
    }

    return id;
}


int kqt_Handle_set_data(
        kqt_Handle handle,
        const char* key,
        const void* data,
        long length)
{
    check_handle(handle, 0);

    Handle* h = get_handle(handle);
    check_data_is_valid(h, 0);
    check_key(h, key, 0);

    // Short-circuit if we have already got invalid data
    // TODO: Remove this if we decide to collect more error info
    if (Error_is_set(&h->validation_error))
        return 1;

    if (length < 0)
    {
        Handle_set_error(
                h, ERROR_ARGUMENT, "Data length must be non-negative");
        return 0;
    }

    if (data == NULL && length > 0)
    {
        Handle_set_error(
                h,
                ERROR_ARGUMENT,
                "Data must not be null if given length (%ld) is positive",
                length);
        return 0;
    }

    if (!parse_data(h, key, data, length))
        return 0;

    h->data_is_validated = false;

    return 1;
}


bool Handle_init(Handle* handle, long buffer_size)
{
    assert(handle != NULL);
    assert(buffer_size > 0);

    handle->data_is_valid = true;
    handle->data_is_validated = true;
    handle->module = NULL;
    handle->error = *ERROR_AUTO;
    handle->validation_error = *ERROR_AUTO;
    memset(handle->position, '\0', POSITION_LENGTH);
    handle->player = NULL;
    handle->length_counter = NULL;

//    int buffer_count = SONG_DEFAULT_BUF_COUNT;
//    int voice_count = 256;

    handle->module = new_Module(buffer_size);
    if (handle->module == NULL)
    {
        Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        Handle_deinit(handle);
        return false;
    }

    // Create players
    handle->player = new_Player(
            handle->module,
            DEFAULT_AUDIO_RATE,
            2048,
            16384,
            256);
    handle->length_counter = new_Player(handle->module, 1000000000L, 0, 0, 0);
    if (handle->player == NULL || handle->length_counter == NULL)
    {
        Handle_set_error(NULL, ERROR_MEMORY, "Couldn't allocate memory");
        Handle_deinit(handle);
        return false;
    }

    Handle_stop(handle);
    //kqt_Handle_set_position(handle, 0, 0);
    return true;
}


const char* kqt_Handle_get_error(kqt_Handle handle)
{
    if (!kqt_Handle_is_valid(handle))
        return Error_get_desc(&null_error);

    Handle* h = get_handle(handle);
    return Error_get_desc(&h->error);
}


void kqt_Handle_clear_error(kqt_Handle handle)
{
    if (!kqt_Handle_is_valid(handle))
    {
        Error_clear(&null_error);
        return;
    }

    Handle* h = get_handle(handle);
    check_data_is_valid_void(h);
    Error_clear(&h->error);

    return;
}


#define set_invalid_if(cond, ...)                           \
    if (true)                                               \
    {                                                       \
        if (cond)                                           \
        {                                                   \
            Handle_set_error(h, ERROR_FORMAT, __VA_ARGS__); \
            h->data_is_valid = false;                       \
            return 0;                                       \
        }                                                   \
    } else (void)0

int kqt_Handle_validate(kqt_Handle handle)
{
    check_handle(handle, 0);
    Handle* h = get_handle(handle);

    check_data_is_valid(h, 0);

    // Check error from set_data
    if (Error_is_set(&h->validation_error))
    {
        Error_copy(&h->error, &h->validation_error);
        Error_copy(&null_error, &h->validation_error);
        h->data_is_valid = false;
        return 0;
    }

    // Check album
    if (h->module->album_is_existent)
    {
        const Track_list* tl = h->module->track_list;
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
        if (!Song_table_get_existent(h->module->songs, i))
            continue;

        // Check for orphans
        const Track_list* tl = h->module->track_list;
        set_invalid_if(
                !h->module->album_is_existent || tl == NULL,
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
        const Order_list* ol = h->module->order_lists[i];
        set_invalid_if(
                ol == NULL || Order_list_get_len(ol) == 0,
                "Song %d does not contain systems", i);

        // Check for missing pattern instances
        for (size_t system = 0; system < Order_list_get_len(ol); ++system)
        {
            const Pat_inst_ref* piref = Order_list_get_pat_inst_ref(ol, system);
            Pattern* pat = Pat_table_get(h->module->pats, piref->pat);

            set_invalid_if(
                    !Pat_table_get_existent(h->module->pats, piref->pat) ||
                    pat == NULL ||
                    !Pattern_get_inst_existent(pat, piref->inst),
                    "Missing pattern instance [%" PRId16 ", %" PRId16 "]",
                    piref->pat, piref->inst);
        }
    }

    // Check for nonexistent songs in the track list
    if (h->module->album_is_existent)
    {
        const Track_list* tl = h->module->track_list;
        assert(tl != NULL);

        for (size_t i = 0; i < Track_list_get_len(tl); ++i)
        {
            set_invalid_if(
                    !Song_table_get_existent(h->module->songs,
                        Track_list_get_song_index(tl, i)),
                    "Album includes nonexistent song %d", i);
        }
    }

    // Check existing patterns
    for (int i = 0; i < KQT_PATTERNS_MAX; ++i)
    {
        if (!Pat_table_get_existent(h->module->pats, i))
            continue;

        Pattern* pat = Pat_table_get(h->module->pats, i);
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
                        !h->module->album_is_existent,
                        "Pattern instance [%d, %d] exists but no album"
                        " is present", i, k);

                bool instance_found = false;

                const Track_list* tl = h->module->track_list;
                assert(tl != NULL);

                for (size_t track = 0; track < Track_list_get_len(tl); ++track)
                {
                    const int song_index = Track_list_get_song_index(tl, track);

                    if (!Song_table_get_existent(
                                h->module->songs,
                                song_index))
                        continue;

                    const Order_list* ol = h->module->order_lists[song_index];
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

    // Check controls
    if (h->module->ins_map != NULL)
    {
        set_invalid_if(
                !Input_map_is_valid(h->module->ins_map, h->module->ins_controls),
                "Control map uses nonexistent controls");
    }

    h->data_is_validated = true;

    return 1;
}

#undef set_invalid_if


void Handle_set_error_(
        Handle* handle,
        Error_type type,
        Error_delay_type delay_type,
        const char* file,
        int line,
        const char* func,
        const char* message,
        ...)
{
    assert(type < ERROR_COUNT_);
    assert(delay_type == ERROR_IMMEDIATE || delay_type == ERROR_VALIDATION);
    assert(file != NULL);
    assert(line >= 0);
    assert(func != NULL);
    assert(message != NULL);

    Error* error = ERROR_AUTO;

    va_list args;
    va_start(args, message);
    Error_set_desc_va_list(error, type, file, line, func, message, args);
    va_end(args);

    if (delay_type == ERROR_IMMEDIATE)
        Error_copy(&null_error, error);

    if (handle != NULL)
    {
        if (delay_type == ERROR_IMMEDIATE)
            Error_copy(&handle->error, error);
        else if (delay_type == ERROR_VALIDATION)
            Error_copy(&handle->validation_error, error);
        else
            assert(false);
    }

    return;
}


void Handle_set_error_from_Error(Handle* handle, const Error* error)
{
    assert(error != NULL);
    assert(Error_is_set(error));

    Error_copy(&null_error, error);

    if (handle != NULL)
        Error_copy(&handle->error, error);

    return;
}


void Handle_set_validation_error_from_Error(Handle* handle, const Error* error)
{
    assert(handle != NULL);
    assert(error != NULL);
    assert(Error_get_type(error) == ERROR_FORMAT);

    Error_copy(&handle->validation_error, error);

    return;
}


bool key_is_valid(Handle* handle, const char* key)
{
    assert(handle != NULL);

    if (key == NULL)
    {
        Handle_set_error(handle, ERROR_ARGUMENT, "No key given");
        return false;
    }

    if (strlen(key) > KQT_KEY_LENGTH_MAX)
    {
        char key_repr[KQT_KEY_LENGTH_MAX + 3] = { '\0' };
        strncpy(key_repr, key, KQT_KEY_LENGTH_MAX - 1);
        strcat(key_repr, "...");
        Handle_set_error(handle, ERROR_ARGUMENT, "Key %s is too long"
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
            Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains an"
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
                Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains"
                        " an invalid component", key);
                return false;
            }
            else if (element_has_period)
            {
                Handle_set_error(handle, ERROR_ARGUMENT, "Key %s contains"
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
        Handle_set_error(handle, ERROR_ARGUMENT, "The final element of"
                " key %s does not have a period", key);
        return false;
    }

    return true;
}


Module* Handle_get_module(Handle* handle)
{
    assert(handle != NULL);
    return handle->module;
}


void Handle_deinit(Handle* handle)
{
    assert(handle != NULL);

    del_Player(handle->length_counter);
    handle->length_counter = NULL;
    del_Player(handle->player);
    handle->player = NULL;

    del_Module(handle->module);
    handle->module = NULL;

    return;
}


void kqt_del_Handle(kqt_Handle handle)
{
    check_handle_void(handle);
    Handle* h = get_handle(handle);

    if (!remove_handle(handle))
    {
        Handle_set_error(NULL, ERROR_ARGUMENT,
                "Invalid Kunquat Handle: %d", handle);
        return;
    }

    Handle_deinit(h);
    memory_free(h);

    return;
}


bool kqt_Handle_is_valid(kqt_Handle handle)
{
    handle -= 1;
    return (handle >= 0) &&
        (handle < KQT_HANDLES_MAX) &&
        (handles[handle] != NULL);
}


