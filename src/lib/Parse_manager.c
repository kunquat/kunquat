

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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <File_base.h>
#include <Handle_private.h>


static bool Parse_song_level(kqt_Handle* handle,
                             const char* key,
                             void* data,
                             long length);


static bool Parse_instrument_level(kqt_Handle* handle,
                                   const char* key,
                                   const char* subkey,
                                   void* data,
                                   long length,
                                   int index);


static bool Parse_pattern_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index);


static bool Parse_scale_level(kqt_Handle* handle,
                              const char* key,
                              const char* subkey,
                              void* data,
                              long length,
                              int index);


static bool Parse_subsong_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index);


static bool is_index_digit(char ch);

static int parse_index(const char* str);


static bool is_index_digit(char ch)
{
    return isxdigit(ch) && (strchr("ABCDEF", ch) == NULL);
}


static int parse_index(const char* str)
{
    int num = 0;
    while (*str != '\0' && *str != '/')
    {
        num *= 16;
        if (!is_index_digit(*str))
        {
            return -1;
        }
        if (isdigit(*str))
        {
            num += (*str - '0');
        }
        else
        {
            char reject[2] = { '\0' };
            reject[0] = *str;
            num += strcspn("abcdef", reject) + 10;
        }
        ++str;
    }
    return num;
}


bool Parse_data(kqt_Handle* handle,
                const char* key,
                void* data,
                long length)
{
    assert(handle != NULL);
    assert(handle->mode != KQT_READ);
    assert(key_is_valid(key));
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (data == NULL)
    {
        return true;
    }
    int last_index = 0;
    const char* last_element = strrchr(key, '/');
    if (last_element == NULL)
    {
        last_element = key;
    }
    else
    {
        ++last_element;
        last_index = last_element - key;
    }
    if (strncmp(last_element, "p_", 2) != 0)
    {
        return true;
    }
    if (last_index == 0)
    {
        return Parse_song_level(handle, key, data, length);
    }
    int first_len = strcspn(key, "/");
    int index = 0;
    const char* second_element = &key[first_len + 1];
    if (strncmp(key, "instrument_", first_len - 2) == 0 &&
            (index = parse_index(&key[first_len - 2])) >= 0)
    {
        return Parse_instrument_level(handle, key, second_element,
                                      data, length, index);
    }
    else if (strncmp(key, "pattern_", first_len - 3) == 0 &&
            (index = parse_index(&key[first_len - 3])) >= 0)
    {
        return Parse_pattern_level(handle, key, second_element,
                                   data, length, index);
    }
    else if (strncmp(key, "scale_", first_len - 1) == 0 &&
            (index = parse_index(&key[first_len - 1])) >= 0)
    {
        return Parse_scale_level(handle, key, second_element,
                                 data, length, index);
    }
    else if (strncmp(key, "subsong_", first_len - 2) == 0 &&
            (index = parse_index(&key[first_len - 2])) >= 0)
    {
        return Parse_subsong_level(handle, key, second_element,
                                   data, length, index);
    }
    return true;
}


static bool Parse_song_level(kqt_Handle* handle,
                             const char* key,
                             void* data,
                             long length)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(data != NULL);
    assert(length >= 0);
    if (strcmp(key, "p_composition.json") == 0)
    {
        Read_state* state = READ_STATE_AUTO;
        if (!Song_parse_composition(handle->song, data, state))
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    "%s: %s", __func__, key, state->message);
            return false;
        }
    }
    return true;
}


static bool Parse_instrument_level(kqt_Handle* handle,
                                   const char* key,
                                   const char* subkey,
                                   void* data,
                                   long length,
                                   int index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert(data != NULL);
    assert(length >= 0);
    if (index < 1 || index > KQT_INSTRUMENTS_MAX)
    {
        return true;
    }
    return true;
}


static bool Parse_pattern_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert(data != NULL);
    assert(length >= 0);
    if (index < 0 || index >= KQT_PATTERNS_MAX)
    {
        return true;
    }
    return true;
}


static bool Parse_scale_level(kqt_Handle* handle,
                              const char* key,
                              const char* subkey,
                              void* data,
                              long length,
                              int index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert(data != NULL);
    assert(length >= 0);
    if (index < 0 || index >= KQT_SCALES_MAX)
    {
        return true;
    }
    return true;
}


static bool Parse_subsong_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert(data != NULL);
    assert(length >= 0);
    if (index < 0 || index >= KQT_SUBSONGS_MAX)
    {
        return true;
    }
    return true;
}


