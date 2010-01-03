

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
#include <ctype.h>

#include <File_base.h>
#include <Handle_private.h>

#include <xmemory.h>


static bool parse_song_level(kqt_Handle* handle,
                             const char* key,
                             void* data,
                             long length);


static bool parse_instrument_level(kqt_Handle* handle,
                                   const char* key,
                                   const char* subkey,
                                   void* data,
                                   long length,
                                   int index);


static bool parse_generator_level(kqt_Handle* handle,
                                  const char* key,
                                  const char* subkey,
                                  void* data,
                                  long length,
                                  int ins_index,
                                  int gen_index);


static bool parse_pattern_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index);


static bool parse_scale_level(kqt_Handle* handle,
                              const char* key,
                              const char* subkey,
                              void* data,
                              long length,
                              int index);


static bool parse_subsong_level(kqt_Handle* handle,
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


int parse_index_dir(const char* key, const char* prefix, int digits)
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


bool parse_data(kqt_Handle* handle,
                const char* key,
                void* data,
                long length)
{
//    fprintf(stderr, "parsing %s\n", key);
    assert(handle != NULL);
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
    char* json = NULL;
    char* json_key = strstr(key, ".json");
    // This comparison doesn't work correctly if we specify a key to be parsed
    // that contains ".json" in the middle. That isn't very likely.
    if (json_key != NULL && strlen(json_key) == 5)
    {
        json = xcalloc(char, length + 1);
        if (json == NULL)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
            return false;
        }
        strncpy(json, data, length);
        data = json;
    }
    if (last_index == 0)
    {
        bool success = parse_song_level(handle, key, data, length);
        xfree(json);
        return success;
    }
    int first_len = strcspn(key, "/");
    int index = 0;
    const char* second_element = &key[first_len + 1];
    bool success = true;
    if (strncmp(key, "instrument_", first_len - 2) == 0 &&
            (index = parse_index(&key[first_len - 2])) >= 0)
    {
        success = parse_instrument_level(handle, key, second_element,
                                         data, length, index);
    }
    else if (strncmp(key, "pattern_", first_len - 3) == 0 &&
            (index = parse_index(&key[first_len - 3])) >= 0)
    {
        success = parse_pattern_level(handle, key, second_element,
                                      data, length, index);
    }
    else if (strncmp(key, "scale_", first_len - 1) == 0 &&
            (index = parse_index(&key[first_len - 1])) >= 0)
    {
        success = parse_scale_level(handle, key, second_element,
                                    data, length, index);
    }
    else if (strncmp(key, "subsong_", first_len - 2) == 0 &&
            (index = parse_index(&key[first_len - 2])) >= 0)
    {
        success = parse_subsong_level(handle, key, second_element,
                                      data, length, index);
    }
    xfree(json);
    return success;
}


static bool parse_song_level(kqt_Handle* handle,
                             const char* key,
                             void* data,
                             long length)
{
//    fprintf(stderr, "song level: %s\n", key);
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


static bool parse_instrument_level(kqt_Handle* handle,
                                   const char* key,
                                   const char* subkey,
                                   void* data,
                                   long length,
                                   int index)
{
 //   fprintf(stderr, "instrument level: %s\n", key);
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
    int gen_index = 0;
    if ((gen_index = parse_index_dir(subkey, "generator_", 2)) >= 0)
    {
        subkey = strchr(subkey, '/');
        assert(subkey != NULL);
        ++subkey;
        return parse_generator_level(handle, key, subkey,
                                     data, length, 
                                     index, gen_index);
    }
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
    struct
    {
        char* name;
        bool (*read)(Instrument_params*, char* str, Read_state*);
    } parse[] =
    {
        { "p_envelope_volume_release.json", Instrument_params_parse_env_vol_rel },
        { NULL, NULL }
    };
    for (int i = 0; parse[i].name != NULL; ++i)
    {
        assert(parse[i].read != NULL);
        if (strcmp(subkey, parse[i].name) == 0)
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
            if (!parse[i].read(Instrument_get_params(ins), data, state))
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
    }
    return true;
}


static bool parse_generator_level(kqt_Handle* handle,
                                  const char* key,
                                  const char* subkey,
                                  void* data,
                                  long length,
                                  int ins_index,
                                  int gen_index)
{
//    fprintf(stderr, "generator level: %s\n", key);
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert(data != NULL || length == 0);
    assert(length >= 0);
    assert(ins_index >= 1);
    assert(ins_index <= KQT_INSTRUMENTS_MAX);
    if (gen_index < 0 || gen_index >= KQT_GENERATORS_MAX)
    {
        return true;
    }
    Instrument* ins = Ins_table_get(Song_get_insts(handle->song), ins_index);
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
    if (strcmp(subkey, "p_generator.json") == 0)
    {
        Generator* common_params = Instrument_get_common_gen_params(ins, gen_index);
        assert(common_params != NULL);
        Read_state* state = READ_STATE_AUTO;
        if (!Generator_parse_general(common_params, data, state))
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    " %s: %s", __func__, key, state->message);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        for (Gen_type i = GEN_TYPE_NONE + 1; i < GEN_TYPE_LAST; ++i)
        {
            Generator* gen = Instrument_get_gen_of_type(ins, gen_index, i);
            if (gen != NULL)
            {
                Generator_copy_general(gen, common_params);
            }
        }
    }
    else if (strcmp(subkey, "p_gen_type.json") == 0)
    {
        Read_state* state = READ_STATE_AUTO;
        Gen_type type = Generator_type_parse(data, state);
        if (state->error)
        {
            kqt_Handle_set_error(handle, "%s: Error in parsing"
                    " %s: %s", __func__, key, state->message);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        Generator* gen = Instrument_get_gen_of_type(ins, gen_index, type);
        if (gen == NULL)
        {
//            fprintf(stderr, "1\n");
            gen = new_Generator(type, Instrument_get_params(ins));
//            fprintf(stderr, "2 -- gen %p for ins %p\n", (void*)gen, (void*)ins);
            if (gen == NULL)
            {
                kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                        __func__);
                if (new_ins)
                {
                    del_Instrument(ins);
                }
                return false;
            }
            Generator* common_params = Instrument_get_common_gen_params(ins, gen_index);
            assert(common_params != NULL);
            Generator_copy_general(gen, common_params);
            Instrument_set_gen(ins, gen_index, gen);
        }
        else
        {
            Instrument_set_gen(ins, gen_index, gen);
        }
    }
    else
    {
        for (Gen_type i = GEN_TYPE_NONE + 1; i < GEN_TYPE_LAST; ++i)
        {
            if (Generator_type_has_subkey(i, subkey))
            {
                Generator* gen = Instrument_get_gen_of_type(ins, gen_index, i);
                bool new_gen = gen == NULL;
                if (new_gen)
                {
                    gen = new_Generator(i, Instrument_get_params(ins));
/*                    if (i == GEN_TYPE_PCM)
                    {
                        fprintf(stderr, "Creating pcm %p for inst %p\n", (void*)gen, (void*)ins);
                    } */
                    if (gen == NULL)
                    {
                        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                                __func__);
                        if (new_ins)
                        {
                            del_Instrument(ins);
                        }
                        return false;
                    }
                    Generator_copy_general(gen,
                            Instrument_get_common_gen_params(ins, gen_index));
                }
                Read_state* state = READ_STATE_AUTO;
                if (!Generator_parse(gen, subkey, data, length, state))
                {
                    kqt_Handle_set_error(handle, "%s: Error in parsing"
                            " %s: %s", __func__, key, state->message);
                    if (new_gen)
                    {
                        del_Generator(gen);
                    }
                    if (new_ins)
                    {
                        del_Instrument(ins);
                    }
                    return false;
                }
                if (new_gen)
                {
                    Instrument_set_gen_of_type(ins, gen_index, gen);
                }
                break;
            }
        }
    }
    if (new_ins && !Ins_table_set(Song_get_insts(handle->song), ins_index, ins))
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        del_Instrument(ins);
        return false;
    }
    return true;
}


static bool parse_pattern_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index)
{
//    fprintf(stderr, "pattern level: %s\n", key);
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


static bool parse_scale_level(kqt_Handle* handle,
                              const char* key,
                              const char* subkey,
                              void* data,
                              long length,
                              int index)
{
//    fprintf(stderr, "scale level: %s\n", key);
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


static bool parse_subsong_level(kqt_Handle* handle,
                                const char* key,
                                const char* subkey,
                                void* data,
                                long length,
                                int index)
{
//    fprintf(stderr, "subsong level: %s\n", key);
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


