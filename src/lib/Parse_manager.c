

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
#include <ctype.h>
#include <inttypes.h>

#include <Connections.h>
#include <Connections_search.h>
#include <File_base.h>
#include <Device_event_keys.h>
#include <Device_params.h>
#include <Gen_type.h>
#include <Handle_private.h>
#include <string_common.h>
#include <xassert.h>
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


static bool parse_dsp_level(kqt_Handle* handle,
                            Instrument* ins,
                            const char* key,
                            const char* subkey,
                            void* data,
                            long length,
                            int dsp_index);


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


static bool key_is_for_text(const char* key);


#define set_parse_error(handle, state) \
    (kqt_Handle_set_error((handle), ERROR_FORMAT, "Parse error in" \
            " %s:%d: %s", (state)->path, (state)->row, (state)->message))


static bool key_is_for_text(const char* key)
{
    assert(key != NULL);
    return string_has_suffix(key, ".json") ||
           key_is_text_device_param(key);
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
    if ((index = string_extract_index(key, "ins_", 2)) >= 0)
    {
        bool changed = Ins_table_get(Song_get_insts(handle->song),
                                                    index) != NULL;
        success = parse_instrument_level(handle, key, second_element,
                                         data, length, index);
        changed ^= Ins_table_get(Song_get_insts(handle->song),
                                                index) != NULL;
        Connections* graph = handle->song->connections;
        if (changed && graph != NULL)
        {
            if (!Connections_prepare(graph,
                                     &handle->song->parent,
                                     Song_get_insts(handle->song),
                                     Song_get_dsps(handle->song)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
            //fprintf(stderr, "line: %d\n", __LINE__);
            //Connections_print(graph, stderr);
        }
    }
    else if ((index = string_extract_index(key, "dsp_", 2)) >= 0)
    {
        bool changed = DSP_table_get_dsp(Song_get_dsps(handle->song),
                                         index) != NULL;
        success = parse_dsp_level(handle, NULL, key, second_element,
                                  data, length, index);
        changed ^= DSP_table_get_dsp(Song_get_dsps(handle->song),
                                     index) != NULL;
        Connections* graph = handle->song->connections;
        if (changed && graph != NULL)
        {
            if (!Connections_prepare(graph,
                                     &handle->song->parent,
                                     Song_get_insts(handle->song),
                                     Song_get_dsps(handle->song)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
        }
    }
    else if ((index = string_extract_index(key, "pat_", 3)) >= 0)
    {
        success = parse_pattern_level(handle, key, second_element,
                                      data, length, index);
    }
    else if ((index = string_extract_index(key, "scale_", 1)) >= 0)
    {
        success = parse_scale_level(handle, key, second_element,
                                    data, length, index);
    }
    else if ((index = string_extract_index(key, "subs_", 2)) >= 0)
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
    if (string_eq(key, "p_composition.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Song_parse_composition(handle->song, data, state))
        {
            set_parse_error(handle, state);
            return false;
        }
    }
    else if (string_eq(key, "p_connections.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Connections* graph = new_Connections_from_string(data, false, state);
        if (graph == NULL)
        {
            if (state->error)
            {
                set_parse_error(handle, state);
            }
            else
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            }
            return false;
        }
        if (handle->song->connections != NULL)
        {
            del_Connections(handle->song->connections);
        }
        handle->song->connections = graph;
        //fprintf(stderr, "line: %d\n", __LINE__);
        //Connections_print(graph, stderr);
        if (!Connections_prepare(graph,
                                 &handle->song->parent,
                                 Song_get_insts(handle->song),
                                 Song_get_dsps(handle->song)))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
        //fprintf(stderr, "line: %d\n", __LINE__);
        //Connections_print(graph, stderr);
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
    int gen_index = -1;
    int dsp_index = -1;
    if ((gen_index = string_extract_index(subkey, "gen_", 2)) >= 0)
    {
        subkey = strchr(subkey, '/');
        assert(subkey != NULL);
        ++subkey;
        Instrument* ins = Ins_table_get(Song_get_insts(handle->song), index);
        bool changed = ins != NULL && Instrument_get_gen(ins,
                                                         gen_index) != NULL;
        bool success = parse_generator_level(handle, key, subkey,
                                             data, length, 
                                             index, gen_index);
        ins = Ins_table_get(Song_get_insts(handle->song), index);
        changed ^= ins != NULL && Instrument_get_gen(ins, gen_index) != NULL;
        Connections* graph = handle->song->connections;
        if (changed && graph != NULL)
        {
//            fprintf(stderr, "instrument %d, generator %d\n", index, gen_index);
            if (!Connections_prepare(graph,
                                     &handle->song->parent,
                                     Song_get_insts(handle->song),
                                     Song_get_dsps(handle->song)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
            //fprintf(stderr, "line: %d\n", __LINE__);
            //Connections_print(graph, stderr);
        }
        return success;
    }
    else if ((dsp_index = string_extract_index(subkey, "dsp_", 2)) >= 0)
    {
        subkey = strchr(subkey, '/');
        assert(subkey != NULL);
        ++subkey;
        Instrument* ins = Ins_table_get(Song_get_insts(handle->song), index);
        bool changed = ins != NULL && Instrument_get_dsp(ins,
                                              dsp_index) != NULL;
        if (ins == NULL)
        {
            ins = new_Instrument(Device_get_buffer_size((Device*)handle->song),
                                 Device_get_mix_rate((Device*)handle->song),
                                 Song_get_scales(handle->song),
                                 Song_get_active_scale(handle->song),
                                 handle->song->random);
            if (ins == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
            if (!Ins_table_set(Song_get_insts(handle->song), index, ins))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_Instrument(ins);
                return false;
            }
        }
//        fprintf(stderr, "creating dsp %d\n", (int)dsp_index);
        bool success = parse_dsp_level(handle, ins, key, subkey,
                                       data, length, dsp_index);
//        fprintf(stderr, "created dsp %p\n", (void*)Instrument_get_dsp(ins, dsp_index));
        changed ^= ins != NULL && Instrument_get_dsp(ins, dsp_index) != NULL;
        Connections* graph = handle->song->connections;
        if (changed && graph != NULL)
        {
            if (!Connections_prepare(graph,
                                     &handle->song->parent,
                                     Song_get_insts(handle->song),
                                     Song_get_dsps(handle->song)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
//            fprintf(stderr, "Set up connections:\n");
//            Connections_print(graph, stderr);
        }
        return success;
    }
    if (string_eq(subkey, "p_instrument.json"))
    {
        Instrument* ins = Ins_table_get(Song_get_insts(handle->song), index);
        bool new_ins = ins == NULL;
        if (new_ins)
        {
            ins = new_Instrument(Device_get_buffer_size((Device*)handle->song),
                                 Device_get_mix_rate((Device*)handle->song),
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
    else if (string_eq(subkey, "p_connections.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Connections* graph = new_Connections_from_string(data, true, state);
        if (graph == NULL)
        {
            if (state->error)
            {
                set_parse_error(handle, state);
            }
            else
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            }
            return false;
        }
        Instrument* ins = Ins_table_get(Song_get_insts(handle->song), index);
        if (ins == NULL)
        {
            ins = new_Instrument(Device_get_buffer_size((Device*)handle->song),
                                 Device_get_mix_rate((Device*)handle->song),
                                 Song_get_scales(handle->song),
                                 Song_get_active_scale(handle->song),
                                 handle->song->random);
            if (ins == NULL || !Ins_table_set(Song_get_insts(handle->song),
                                              index, ins))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_Connections(graph);
                return false;
            }
        }
        assert(ins != NULL);
        Instrument_set_connections(ins, graph);
//        fprintf(stderr, "Set connections for ins %d\n", index);
        Connections* global_graph = handle->song->connections;
        if (global_graph != NULL)
        {
            if (!Connections_prepare(global_graph,
                                     &handle->song->parent,
                                     Song_get_insts(handle->song),
                                     Song_get_dsps(handle->song)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                return false;
            }
//            fprintf(stderr, "line: %d\n", __LINE__);
//            Connections_print(global_graph, stderr);
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
        if (string_eq(subkey, parse[i].name))
        {
            Instrument* ins = Ins_table_get(Song_get_insts(handle->song), index);
            bool new_ins = ins == NULL;
            if (new_ins)
            {
                ins = new_Instrument(Device_get_buffer_size((Device*)handle->song),
                                     Device_get_mix_rate((Device*)handle->song),
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
        ins = new_Instrument(Device_get_buffer_size((Device*)handle->song),
                             Device_get_mix_rate((Device*)handle->song),
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
    if (string_eq(subkey, "p_gen_type.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Generator* gen = new_Generator(data, Instrument_get_params(ins),
                                 Device_get_buffer_size((Device*)handle->song),
                                 Device_get_mix_rate((Device*)handle->song),
                                 handle->song->random,
                                 state);
        if (gen == NULL)
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

        char* (*property)(Generator*, const char*) =
                Gen_type_find_property(Generator_get_type(gen));
        if (property != NULL)
        {
            char* size_str = property(gen, "voice_state_size");
            if (size_str != NULL)
            {
                Read_state* state = READ_STATE_AUTO;
                int64_t size = 0;
                read_int(size_str, &size, state);
                assert(!state->error);
                assert(size >= 0);
//                fprintf(stderr, "Reserving space for %" PRId64 " bytes\n",
//                                size);
                if (!Voice_pool_reserve_state_space(
                            handle->song->play_state->voice_pool, size))
                {
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                    del_Generator(gen);
                    if (new_ins)
                    {
                        del_Instrument(ins);
                    }
                    return false;
                }
            }
        }
        
        Gen_table* table = Instrument_get_gens(ins);
        assert(table != NULL);
        if (!Gen_table_set_gen(table, gen_index, gen))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            del_Generator(gen);
            if (new_ins)
            {
                del_Instrument(ins);
            }
            return false;
        }
    }
    else if (string_eq(subkey, "p_events.json"))
    {
        Gen_table* table = Instrument_get_gens(ins);
        assert(table != NULL);
        Gen_conf* conf = Gen_table_get_conf(table, gen_index);
        if (conf == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Device_params_parse_events(conf->params,
                                        DEVICE_EVENT_TYPE_GENERATOR,
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
    }
    else
    {
        Gen_table* table = Instrument_get_gens(ins);
        assert(table != NULL);
        Gen_conf* conf = Gen_table_get_conf(table, gen_index);
        if (conf == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Gen_conf_parse(conf, subkey, data, length, state))
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


static bool parse_dsp_level(kqt_Handle* handle,
                            Instrument* ins,
                            const char* key,
                            const char* subkey,
                            void* data,
                            long length,
                            int dsp_index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    if (dsp_index < 0 || dsp_index >= KQT_DSP_EFFECTS_MAX)
    {
        return true;
    }
    if (!string_has_prefix(subkey, MAGIC_ID "eXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "e" KQT_FORMAT_VERSION "/"))
    {
        return true;
    }
    subkey = strchr(subkey, '/') + 1;
    if (string_eq(subkey, "p_dsp_type.json"))
    {
//        fprintf(stderr, "%s\n", subkey);
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        DSP* dsp = new_DSP(data,
                           Device_get_buffer_size((Device*)handle->song),
                           Device_get_mix_rate((Device*)handle->song),
                           state);
        if (dsp == NULL)
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
        DSP_table* table = ins != NULL ? Instrument_get_dsps(ins) :
                                         Song_get_dsps(handle->song);
        assert(table != NULL);
        if (!DSP_table_set_dsp(table, dsp_index, dsp))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            del_DSP(dsp);
            return false;
        }
    }
    else if ((string_has_prefix(subkey, "i/") ||
              string_has_prefix(subkey, "c/")) &&
             key_is_device_param(subkey))
    {
//        fprintf(stderr, "%s\n", subkey);
        DSP_table* table = ins != NULL ? Instrument_get_dsps(ins) :
                                         Song_get_dsps(handle->song);
        assert(table != NULL);
        DSP_conf* conf = DSP_table_get_conf(table, dsp_index);
        if (conf == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!DSP_conf_parse(conf, subkey, data, length, state))
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
    }
    else if (string_eq(subkey, "p_events.json"))
    {
//        fprintf(stderr, "%s\n", subkey);
        DSP_table* table = ins != NULL ? Instrument_get_dsps(ins) :
                                         Song_get_dsps(handle->song);
        assert(table != NULL);
        DSP_conf* conf = DSP_table_get_conf(table, dsp_index);
        if (conf == NULL)
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Device_params_parse_events(conf->params,
                                        DEVICE_EVENT_TYPE_DSP,
                                        handle->song->event_handler,
                                        data,
                                        state))
        {
            set_parse_error(handle, state);
            return false;
        }
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
    if (string_eq(subkey, "p_pattern.json"))
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
    bool global_column = string_eq(subkey, "gcol/p_global_events.json");
    int col_index = 0;
    ++second_element;
    if (((col_index = string_extract_index(subkey, "ccol_", 2)) >= 0
                    && string_eq(second_element, "p_channel_events.json"))
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
    if (string_eq(subkey, MAGIC_ID "sXX/p_scale.json") ||
            string_eq(subkey, MAGIC_ID "s" KQT_FORMAT_VERSION "/p_scale.json"))
    {
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
    if (string_eq(subkey, "p_subsong.json"))
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


