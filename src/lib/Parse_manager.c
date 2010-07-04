

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
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <File_base.h>
#include <Generator_event_keys.h>
#include <Generator_params.h>
#include <Handle_private.h>
#include <string_common.h>

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


static bool key_is_for_text(const char* key);


#define set_parse_error(handle, state) \
    (kqt_Handle_set_error((handle), ERROR_FORMAT, "Parse error in" \
            " %s:%d: %s", (state)->path, (state)->row, (state)->message))


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


static bool key_is_for_text(const char* key)
{
    assert(key != NULL);
    return string_has_suffix(key, ".json") ||
           key_is_text_generator_param(key);
}


bool parse_data(kqt_Handle* handle,
                const char* key,
                void* data,
                long length)
{
//    fprintf(stderr, "parsing %s\n", key);
    assert(handle != NULL);
    check_key(handle, key, false);
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
    if (data != NULL && key_is_for_text(key))
    {
        json = xcalloc(char, length + 1);
        if (json == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
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
    if (strncmp(key, "ins_", first_len - 2) == 0 &&
            (index = parse_index(&key[first_len - 2])) >= 0)
    {
        success = parse_instrument_level(handle, key, second_element,
                                         data, length, index);
    }
    else if (strncmp(key, "pat_", first_len - 3) == 0 &&
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
    else if (strncmp(key, "subs_", first_len - 2) == 0 &&
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
    (void)length;
    if (strcmp(key, "p_composition.json") == 0)
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Song_parse_composition(handle->song, data, state))
        {
            set_parse_error(handle, state);
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
//    fprintf(stderr, "instrument level: %s\n", key);
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    if (index < 0 || index >= KQT_INSTRUMENTS_MAX)
    {
        return true;
    }
    if (!string_has_prefix(subkey, MAGIC_ID "iXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "i" KQT_FORMAT_VERSION "/"))
    {
        return true;
    }
    subkey = strchr(subkey, '/');
    assert(subkey != NULL);
    ++subkey;
    int gen_index = 0;
    if ((gen_index = parse_index_dir(subkey, "gen_", 2)) >= 0)
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
                                 handle->song->random);
            if (ins == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Instrument_parse_header(ins, data, state))
        {
            set_parse_error(handle, state);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        if (new_ins && !Ins_table_set(Song_get_insts(handle->song), index, ins))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
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
        { "p_envelope_force.json", Instrument_params_parse_env_force },
        { "p_envelope_force_release.json", Instrument_params_parse_env_force_rel },
        { "p_envelope_force_filter.json", Instrument_params_parse_env_force_filter },
        { "p_envelope_pitch_pan.json", Instrument_params_parse_env_pitch_pan },
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
                                     handle->song->random);
                if (ins == NULL)
                {
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                    return false;
                }
            }
            Read_state* state = Read_state_init(READ_STATE_AUTO, key);
            if (!parse[i].read(Instrument_get_params(ins), data, state))
            {
                if (!state->error)
                {
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                }
                else
                {
                    set_parse_error(handle, state);
                }
                if (new_ins)
                {
                    del_Instrument(ins);
                }
                return false;
            }
            if (new_ins && !Ins_table_set(Song_get_insts(handle->song), index, ins))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
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
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(ins_index >= 0);
    assert(ins_index < KQT_INSTRUMENTS_MAX);
    if (gen_index < 0 || gen_index >= KQT_GENERATORS_MAX)
    {
        return true;
    }
    if (!string_has_prefix(subkey, MAGIC_ID "gXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "g" KQT_FORMAT_VERSION "/"))
    {
        return true;
    }
    subkey = strchr(subkey, '/');
    ++subkey;
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
                             handle->song->random);
        if (ins == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
    }
    if (strcmp(subkey, "p_generator.json") == 0)
    {
        Generator* common_params = Instrument_get_common_gen_params(ins, gen_index);
        assert(common_params != NULL);
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Generator_parse_general(common_params, data, state))
        {
            set_parse_error(handle, state);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        Generator* gen = Instrument_get_gen(ins, gen_index);
        if (gen != NULL)
        {
            Generator_copy_general(gen, common_params);
        }
#if 0
        for (Gen_type i = GEN_TYPE_NONE + 1; i < GEN_TYPE_LAST; ++i)
        {
            Generator* gen = Instrument_get_gen_of_type(ins, gen_index, i);
            if (gen != NULL)
            {
                Generator_copy_general(gen, common_params);
            }
        }
#endif
    }
    else if ((string_has_prefix(subkey, "i/") ||
              string_has_prefix(subkey, "c/")) &&
             key_is_generator_param(subkey))
    {
        Generator* common_params = Instrument_get_common_gen_params(ins, gen_index);
        assert(common_params != NULL);
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Generator_parse_param(common_params, subkey, data, length, state))
        {
            set_parse_error(handle, state);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        Generator* gen = Instrument_get_gen(ins, gen_index);
        if (gen != NULL)
        {
            Generator_copy_general(gen, common_params);
        }
#if 0
        for (Gen_type i = GEN_TYPE_NONE + 1; i < GEN_TYPE_LAST; ++i)
        {
            Generator* gen = Instrument_get_gen_of_type(ins, gen_index, i);
            if (gen != NULL)
            {
                Generator_copy_general(gen, common_params);
            }
        }
#endif
    }
    else if (strcmp(subkey, "p_events.json") == 0)
    {
        Generator* common_params = Instrument_get_common_gen_params(ins, gen_index);
        assert(common_params != NULL);
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Generator_params_parse_events(common_params->type_params,
                                           handle->song->event_handler,
                                           data,
                                           state))
        {
            set_parse_error(handle, state);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        Generator* gen = Instrument_get_gen(ins, gen_index);
        if (gen != NULL)
        {
            Generator_copy_general(gen, common_params);
        }
#if 0
        for (Gen_type i = GEN_TYPE_NONE + 1; i < GEN_TYPE_LAST; ++i)
        {
            Generator* gen = Instrument_get_gen_of_type(ins, gen_index, i);
            if (gen != NULL)
            {
                Generator_copy_general(gen, common_params);
            }
        }
#endif
    }
    else if (strcmp(subkey, "p_gen_type.json") == 0)
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Gen_type type = Generator_type_parse(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
        Generator* gen = Instrument_get_gen(ins, gen_index);
        if (gen == NULL || Generator_get_type(gen) != type)
        {
            Generator* common_params =
                    Instrument_get_common_gen_params(ins, gen_index);
            assert(common_params != NULL);
            gen = new_Generator(type, Instrument_get_params(ins),
                                Generator_get_params(common_params));
            if (gen == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                if (new_ins)
                {
                    del_Instrument(ins);
                }
                return false;
            }
            Generator_copy_general(gen, common_params);
        }
        Instrument_set_gen(ins, gen_index, gen);
#if 0
        Generator* gen = Instrument_get_gen_of_type(ins, gen_index, type);
        if (gen == NULL)
        {
//            fprintf(stderr, "1\n");
            Generator* common_params = Instrument_get_common_gen_params(ins, gen_index);
            assert(common_params != NULL);
            gen = new_Generator(type, Instrument_get_params(ins),
                                Generator_get_params(common_params));
//            fprintf(stderr, "2 -- gen %p for ins %p\n", (void*)gen, (void*)ins);
            if (gen == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                if (new_ins)
                {
                    del_Instrument(ins);
                }
                return false;
            }
            Generator_copy_general(gen, common_params);
            Instrument_set_gen(ins, gen_index, gen);
        }
        else
        {
            Instrument_set_gen(ins, gen_index, gen);
        }
#endif
    }
    if (new_ins && !Ins_table_set(Song_get_insts(handle->song), ins_index, ins))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory");
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
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    (void)length;
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
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Pattern_parse_header(pat, data, state))
        {
            set_parse_error(handle, state);
            if (new_pattern)
            {
                del_Pattern(pat);
            }
            return false;
        }
        if (new_pattern && !Pat_table_set(Song_get_pats(handle->song), index, pat))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
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
    bool global_column = strcmp(subkey, "gcol/p_global_events.json") == 0;
    int col_index = 0;
    ++second_element;
    if (((col_index = parse_index_dir(subkey, "ccol_", 2)) >= 0
                    && strcmp(second_element, "p_channel_events.json") == 0)
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
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Column* col = new_Column_from_string(Pattern_get_length(pat),
                                             data,
                                             global_column,
                                             state);
        if (col == NULL)
        {
            if (!state->error)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            }
            else
            {
                set_parse_error(handle, state);
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
            if (!Pattern_set_col(pat, col_index, col))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                if (new_pattern)
                {
                    del_Pattern(pat);
                }
                return false;
            }
        }
        if (new_pattern)
        {
            if (!Pat_table_set(Song_get_pats(handle->song), index, pat))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
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
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    (void)length;
    if (index < 0 || index >= KQT_SCALES_MAX)
    {
        return true;
    }
    if (strcmp(subkey, MAGIC_ID "sXX/p_scale.json") == 0 ||
            strcmp(subkey, MAGIC_ID "s" KQT_FORMAT_VERSION "/p_scale.json") == 0)
    {
#if 0
        Scale* scale = new_Scale(SCALE_DEFAULT_REF_PITCH,
                                 SCALE_DEFAULT_OCTAVE_RATIO);
        if (scale == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Scale_parse(scale, data, state))
        {
            set_parse_error(handle, state);
            del_Scale(scale);
            return false;
        }
#endif
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Scale* scale = new_Scale_from_string(data, state);
        if (scale == NULL)
        {
            set_parse_error(handle, state);
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
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    (void)length;
    if (index < 0 || index >= KQT_SUBSONGS_MAX)
    {
        return true;
    }
    if (strcmp(subkey, "p_subsong.json") == 0)
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Subsong* ss = new_Subsong_from_string(data, state);
        if (ss == NULL)
        {
            if (!state->error)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            }
            else
            {
                set_parse_error(handle, state);
            }
            return false;
        }
        Subsong_table* st = Song_get_subsongs(handle->song);
        if (!Subsong_table_set(st, index, ss))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            del_Subsong(ss);
            return false;
        }
    }
    return true;
}


