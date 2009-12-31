

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

static int parse_index_dir(const char* key, const char* prefix, int digits);


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


static int parse_index_dir(const char* key, const char* prefix, int digits)
{
    assert(key != NULL);
    assert(prefix != NULL);
    assert(digits >= 1);
    int prefix_len = strlen(prefix);
    if (strncmp(key, prefix, prefix_len) != 0)
    {
        return -1;
    }
    if ((int)strlen(key) <= prefix_len + digits ||
            key[prefix_len + digits] != '/')
    {
        return -1;
    }
    return parse_index(&key[prefix_len]);
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
    if (length == 0)
    {
        data = NULL;
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
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (strcmp(key, "p_composition.json") == 0)
    {
        Read_state* state = READ_STATE_AUTO;
        if (!Song_parse_composition(handle->song, data, state))
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    " %s: %s", __func__, key, state->message);
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
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (index < 1 || index > KQT_INSTRUMENTS_MAX)
    {
        return true;
    }
    if (strncmp(subkey, "kunquatiXX/", 11) != 0 &&
            strncmp(subkey, "kunquati" KQT_FORMAT_VERSION "/", 11) != 0)
    {
        return true;
    }
    subkey = strchr(subkey, '/');
    assert(subkey != NULL);
    ++subkey;
    if (strcmp(subkey, "p_instrument.json") == 0)
    {
        Instrument* ins = Ins_table_get(Song_get_insts(handle->song), index);
        bool new_ins = ins == NULL;
        if (new_ins)
        {
            ins = new_Instrument(Song_get_bufs(handle->song),
                                 Song_get_voice_bufs(handle->song),
                                 Song_get_voice_bufs2(handle->song),
                                 Song_get_buf_count(handle->song),
                                 Song_get_buf_size(handle->song),
                                 Song_get_scales(handle->song),
                                 Song_get_active_scale(handle->song),
                                 32); // TODO: make configurable
            if (ins == NULL)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                return false;
            }
        }
        Read_state* state = READ_STATE_AUTO;
        if (!Instrument_parse_header(ins, data, state))
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    " %s: %s", __func__, key, state->message);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        if (new_ins && !Ins_table_set(Song_get_insts(handle->song), index, ins))
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            del_Instrument(ins);
            return false;
        }
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
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (index < 0 || index >= KQT_PATTERNS_MAX)
    {
        return true;
    }
    if (strcmp(subkey, "p_pattern.json") == 0)
    {
        Pattern* pat = Pat_table_get(Song_get_pats(handle->song), index);
        bool new_pattern = pat == NULL;
        if (new_pattern)
        {
            pat = new_Pattern();
            if (pat == NULL)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                return false;
            }
        }
        Read_state* state = READ_STATE_AUTO;
        if (!Pattern_parse_header(pat, data, state))
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    " %s: %s", __func__, key, state->message);
            if (new_pattern)
            {
                del_Pattern(pat);
            }
            return false;
        }
        if (new_pattern && !Pat_table_set(Song_get_pats(handle->song), index, pat))
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            del_Pattern(pat);
            return false;
        }
        return true;
    }
    char* second_element = strchr(subkey, '/');
    if (second_element == NULL)
    {
        return true;
    }
    bool global_column = strcmp(subkey, "global_column/p_global_events.json") == 0;
    int col_index = 0;
    ++second_element;
    if (((col_index = parse_index_dir(subkey, "voice_column_", 2)) >= 0
                    && strcmp(second_element, "p_voice_events.json") == 0)
                || global_column)
    {
        if (global_column)
        {
            col_index = -1;
        }
        if (col_index < -1 || col_index >= KQT_COLUMNS_MAX)
        {
            return true;
        }
        Pattern* pat = Pat_table_get(Song_get_pats(handle->song), index);
        bool new_pattern = pat == NULL;
        if (new_pattern)
        {
            pat = new_Pattern();
            if (pat == NULL)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                return false;
            }
        }
        Read_state* state = READ_STATE_AUTO;
        Column* col = new_Column_from_string(Pattern_get_length(pat),
                                             data,
                                             global_column,
                                             state);
        if (col == NULL)
        {
            if (!state->error)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
            }
            else
            {
                kqt_Handle_set_error(handle, "%s: Error in parsing"
                        " %s: %s", __func__, key, state->message);
            }
            if (new_pattern)
            {
                del_Pattern(pat);
            }
            return false;
        }
        if (global_column)
        {
            Pattern_set_global(pat, col);
        }
        else
        {
            Pattern_set_col(pat, col_index, col);
        }
        if (new_pattern)
        {
            if (!Pat_table_set(Song_get_pats(handle->song), index, pat))
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                del_Pattern(pat);
                return false;
            }
        }
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
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (index < 0 || index >= KQT_SCALES_MAX)
    {
        return true;
    }
    if (strcmp(subkey, "kunquatsXX/p_scale.json") == 0 ||
            strcmp(subkey, "kunquats" KQT_FORMAT_VERSION "/p_scale.json") == 0)
    {
        Scale* scale = new_Scale(SCALE_DEFAULT_REF_PITCH,
                                 SCALE_DEFAULT_OCTAVE_RATIO);
        if (scale == NULL)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            return false;
        }
        Read_state* state = READ_STATE_AUTO;
        if (!Scale_parse(scale, data, state))
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    " %s: %s", __func__, key, state->message);
            del_Scale(scale);
            return false;
        }
        Song_set_scale(handle->song, index, scale);
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
    assert(data != NULL || length == 0);
    assert(length >= 0);
    if (index < 0 || index >= KQT_SUBSONGS_MAX)
    {
        return true;
    }
    if (strcmp(subkey, "p_subsong.json") == 0)
    {
        Read_state* state = READ_STATE_AUTO;
        Subsong* ss = new_Subsong_from_string(data, state);
        if (ss == NULL)
        {
            if (!state->error)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
            }
            else
            {
                kqt_Handle_set_error(handle, "%s: Error in parsing"
                        " %s: %s", __func__, key, state->message);
            }
            return false;
        }
        Subsong_table* st = Song_get_subsongs(handle->song);
        if (!Subsong_table_set(st, index, ss))
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            del_Subsong(ss);
            return false;
        }
    }
    return true;
}


