

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
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include <Bind.h>
#include <Connections.h>
#include <Connections_search.h>
#include <Device_event_keys.h>
#include <Device_params.h>
#include <DSP_type.h>
#include <Environment.h>
#include <File_base.h>
#include <Gen_type.h>
#include <Handle_private.h>
#include <manifest.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


static bool parse_module_level(
        kqt_Handle* handle,
        const char* key,
        void* data,
        long length);


static bool parse_album_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length);


static bool parse_instrument_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int index);


static bool parse_effect_level(
        kqt_Handle* handle,
        Instrument* ins,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int eff_index);


static bool parse_generator_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int ins_index,
        int gen_index);


static bool parse_dsp_level(
        kqt_Handle* handle,
        Effect* eff,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int dsp_index);


static bool parse_pattern_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int index);


static bool parse_pat_inst_level(
        kqt_Handle* handle,
        Pattern* pat,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int index);


static bool parse_scale_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int index);


static bool parse_subsong_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int index);


static bool key_is_for_text(const char* key);


#define set_parse_error(handle, state) \
    (kqt_Handle_set_validation_error((handle), ERROR_FORMAT, "Parse error in" \
            " %s:%d: %s", (state)->path, (state)->row, (state)->message))


static bool key_is_for_text(const char* key)
{
    assert(key != NULL);
    return string_has_suffix(key, ".json") ||
           key_is_text_device_param(key);
}


static bool prepare_connections(kqt_Handle* handle)
{
    assert(handle != NULL);

    Module* module = Handle_get_module(handle);
    Connections* graph = module->connections;

    Device_states* states = Player_get_device_states(handle->player);

    if (!Connections_prepare(graph, states))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory for connections");
        return false;
    }

    return true;
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
        json = memory_calloc_items(char, length + 1);
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
        bool success = parse_module_level(handle, key, data, length);
        memory_free(json);
        return success;
    }

    Module* module = Handle_get_module(handle);

    int first_len = strcspn(key, "/");
    int index = 0;
    const char* second_element = &key[first_len + 1];
    bool success = true;
    if ((index = string_extract_index(key, "ins_", 2, "/")) >= 0)
    {
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        bool changed = ins != NULL;
        success = parse_instrument_level(handle, key, second_element,
                                         data, length, index);
        changed ^= Ins_table_get(Module_get_insts(module), index) != NULL;
        Connections* graph = module->connections;
        if (changed && graph != NULL)
        {
            if (!prepare_connections(handle))
                return false;
            //fprintf(stderr, "line: %d\n", __LINE__);
            //Connections_print(graph, stderr);
        }
    }
    else if ((index = string_extract_index(key, "eff_", 2, "/")) >= 0)
    {
        Effect* eff = Effect_table_get(Module_get_effects(module), index);
        bool changed = eff != NULL;
        success = parse_effect_level(handle, NULL, key, second_element,
                                     data, length, index);
        changed ^= Effect_table_get(Module_get_effects(module),
                                                     index) != NULL;
        Connections* graph = module->connections;
        if (changed && graph != NULL)
        {
            if (!prepare_connections(handle))
                return false;
        }
    }
    else if ((index = string_extract_index(key, "pat_", 3, "/")) >= 0)
    {
        Pattern* pat = Pat_table_get(Module_get_pats(module), index);
        success = parse_pattern_level(handle, key, second_element,
                                      data, length, index);
        Pattern* new_pat = Pat_table_get(Module_get_pats(module), index);
        if (success && pat != new_pat && new_pat != NULL)
        {
            // Update pattern location information
            // This is needed for correct jump counter updates
            // when a Pattern with jumps is used multiple times.
            for (int subsong = 0; subsong < KQT_SONGS_MAX; ++subsong)
            {
                const Order_list* ol = module->order_lists[subsong];
                if (ol == NULL)
                    continue;

                const size_t ol_len = Order_list_get_len(ol);
                for (size_t system = 0; system < ol_len; ++system)
                {
                    Pat_inst_ref* ref = Order_list_get_pat_inst_ref(ol, system);
                    if (ref->pat == index)
                    {
                        if (!Pattern_set_location(new_pat, subsong, ref))
                        {
                            kqt_Handle_set_error(handle, ERROR_MEMORY,
                                    "Couldn't allocate memory");
                            return false;
                        }
                    }
                }
#if 0
                Subsong* ss = Subsong_table_get_hidden(
                                      Module_get_subsongs(module),
                                      subsong);
                if (ss == NULL)
                {
                    continue;
                }
                for (int section = 0; section < KQT_SECTIONS_MAX; ++section)
                {
                    if (Subsong_get(ss, section) == index)
                    {
                        if (!Pattern_set_location(new_pat, subsong, section))
                        {
                            kqt_Handle_set_error(handle, ERROR_MEMORY,
                                    "Couldn't allocate memory");
                            return false;
                        }
                    }
                }
#endif
            }
        }
    }
    else if ((index = string_extract_index(key, "scale_", 1, "/")) >= 0)
    {
        success = parse_scale_level(handle, key, second_element,
                                    data, length, index);
    }
    else if ((index = string_extract_index(key, "song_", 2, "/")) >= 0)
    {
        success = parse_subsong_level(handle, key, second_element,
                                      data, length, index);
    }
    else if (string_has_prefix(key, "album/"))
    {
        success = parse_album_level(handle, key, second_element, data, length);
    }
    memory_free(json);
    return success;
}


static bool parse_module_level(kqt_Handle* handle,
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

    Module* module = Handle_get_module(handle);

    if (string_eq(key, "p_composition.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Module_parse_composition(module, data, state))
        {
            set_parse_error(handle, state);
            return false;
        }
    }
    else if (string_eq(key, "p_connections.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Connections* graph = new_Connections_from_string(data,
                                            CONNECTION_LEVEL_GLOBAL,
                                            Module_get_insts(module),
                                            Module_get_effects(module),
                                            NULL,
                                            (Device*)module,
                                            state);
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
        if (module->connections != NULL)
        {
            del_Connections(module->connections);
        }
        module->connections = graph;
        //fprintf(stderr, "line: %d\n", __LINE__);
        //Connections_print(graph, stderr);
        if (!prepare_connections(handle))
            return false;
        //fprintf(stderr, "line: %d\n", __LINE__);
        //Connections_print(graph, stderr);
    }
    else if (string_eq(key, "p_random_seed.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Module_parse_random_seed(module, data, state))
        {
            set_parse_error(handle, state);
            return false;
        }
    }
    else if (string_eq(key, "p_environment.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Environment_parse(module->env, data, state))
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
    else if (string_eq(key, "p_bind.json"))
    {
#if 0
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Bind* map = new_Bind(data,
                        Event_handler_get_names(module->event_handler),
                        state);
        if (map == NULL)
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
        if (!Module_set_bind(module, map))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
#endif
    }
    return true;
}


static bool parse_album_level(
        kqt_Handle* handle,
        const char* key,
        const char* subkey,
        void* data,
        long length)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    (void)length;

    Module* module = Handle_get_module(handle);

    if (string_eq(subkey, "p_manifest.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }
        module->album_is_existent = existent;
    }
    else if (string_eq(subkey, "p_tracks.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Track_list* tl = new_Track_list(data, state);
        if (tl == NULL)
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
        del_Track_list(module->track_list);
        module->track_list = tl;
    }
    return true;
}


static Instrument* add_instrument(kqt_Handle* handle, int index)
{
    assert(handle != NULL);
    assert(index >= 0);
    assert(index < KQT_INSTRUMENTS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new instrument";

    Module* module = Handle_get_module(handle);

    // Return existing instrument
    Instrument* ins = Ins_table_get(Module_get_insts(module), index);
    if (ins != NULL)
        return ins;

    // Create new instrument
    ins = new_Instrument(
            Device_get_buffer_size((Device*)module),
            Device_get_mix_rate((Device*)module));
    if (ins == NULL || !Ins_table_set(Module_get_insts(module), index, ins))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_Instrument(ins);
        return NULL;
    }

    // Allocate Device state(s) for the new Instrument
    Device_state* ds = Device_create_state((Device*)ins);
    if (ds == NULL || !Device_states_add_state(
                Player_get_device_states(handle->player), ds))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        Ins_table_remove(Module_get_insts(module), index);
        return NULL;
    }

    return ins;
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
#if 0
    if (!string_has_prefix(subkey, MAGIC_ID "iXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "i" KQT_FORMAT_VERSION "/"))
    {
        return true;
    }
    subkey = strchr(subkey, '/');
    assert(subkey != NULL);
    ++subkey;
#endif
    int gen_index = -1;
    int eff_index = -1;

    Module* module = Handle_get_module(handle);

    // Subdevices
    if ((gen_index = string_extract_index(subkey, "gen_", 2, "/")) >= 0)
    {
        subkey = strchr(subkey, '/');
        assert(subkey != NULL);
        ++subkey;
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        Generator* gen = ins != NULL ? Instrument_get_gen(ins, gen_index)
                                     : NULL;
        bool changed = ins != NULL && gen != NULL;
        bool success = parse_generator_level(handle, key, subkey,
                                             data, length, 
                                             index, gen_index);
        ins = Ins_table_get(Module_get_insts(module), index);
        gen = ins != NULL ? Instrument_get_gen(ins, gen_index) : NULL;
        changed ^= ins != NULL && gen != NULL;
        Connections* graph = module->connections;
        if (changed && graph != NULL)
        {
//            fprintf(stderr, "instrument %d, generator %d\n", index, gen_index);
            if (!prepare_connections(handle))
                return false;
            //fprintf(stderr, "line: %d\n", __LINE__);
            //Connections_print(graph, stderr);
        }
        return success;
    }
    else if ((eff_index = string_extract_index(subkey, "eff_", 2, "/")) >= 0)
    {
        subkey = strchr(subkey, '/');
        assert(subkey != NULL);
        ++subkey;
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        bool changed = ins != NULL && Instrument_get_effect(ins,
                                                eff_index) != NULL;

        ins = add_instrument(handle, index);
        if (ins == NULL)
            return false;

        bool success = parse_effect_level(handle, ins, key, subkey,
                                          data, length, eff_index);
        changed ^= ins != NULL &&
                   Instrument_get_effect(ins, eff_index) != NULL;
        Connections* graph = module->connections;
        if (changed && graph != NULL)
        {
            if (!prepare_connections(handle))
                return false;
        }
        return success;
    }

    // Instrument data
    if (string_eq(subkey, "p_manifest.json"))
    {
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        ins = add_instrument(handle, index);
        if (ins == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }

        Device_set_existent((Device*)ins, existent);
    }
    else if (string_eq(subkey, "p_instrument.json"))
    {
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        ins = add_instrument(handle, index);
        if (ins == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Instrument_parse_header(ins, data, state))
        {
            set_parse_error(handle, state);
            return false;
        }
    }
    else if (string_eq(subkey, "p_connections.json"))
    {
        bool reconnect = false;
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        if (data == NULL)
        {
            if (ins != NULL)
            {
                Instrument_set_connections(ins, NULL);
                reconnect = true;
            }
        }
        else
        {
            ins = add_instrument(handle, index);
            if (ins == NULL)
                return false;

            Read_state* state = Read_state_init(READ_STATE_AUTO, key);
            Connections* graph = new_Connections_from_string(data,
                                                 CONNECTION_LEVEL_INSTRUMENT,
                                                 Module_get_insts(module),
                                                 Instrument_get_effects(ins),
                                                 NULL,
                                                 (Device*)ins,
                                                 state);
            if (graph == NULL)
            {
                if (state->error)
                    set_parse_error(handle, state);
                else
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                return false;
            }
            Instrument_set_connections(ins, graph);
            reconnect = true;
        }
        if (reconnect)
        {
//            fprintf(stderr, "Set connections for ins %d\n", index);
            Connections* global_graph = module->connections;
            if (global_graph != NULL)
            {
                if (!prepare_connections(handle))
                    return false;
//                fprintf(stderr, "line: %d\n", __LINE__);
//                Connections_print(global_graph, stderr);
            }
        }
    }
    else if (string_has_prefix(subkey, "p_pitch_lock_"))
    {
        Instrument* ins = Ins_table_get(Module_get_insts(module), index);
        ins = add_instrument(handle, index);
        if (ins == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Instrument_parse_value(ins, subkey, data, state))
        {
            set_parse_error(handle, state);
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
        if (string_eq(subkey, parse[i].name))
        {
            Instrument* ins = Ins_table_get(Module_get_insts(module), index);
            ins = add_instrument(handle, index);
            if (ins == NULL)
                return false;

            Read_state* state = Read_state_init(READ_STATE_AUTO, key);
            if (!parse[i].read(Instrument_get_params(ins), data, state))
            {
                if (!state->error)
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                else
                    set_parse_error(handle, state);
                return false;
            }
        }
    }
    return true;
}


static Gen_conf* add_gen_conf(
        kqt_Handle* handle,
        Gen_table* gen_table,
        int gen_index)
{
    assert(handle != NULL);
    assert(gen_table != NULL);
    assert(gen_index >= 0);
    assert(gen_index < KQT_GENERATORS_MAX);

    // Add Generator configuration
    Gen_conf* conf = Gen_table_add_conf(gen_table, gen_index);
    if (conf == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory for generator configuration");
        return NULL;
    }

    // Sync the Generator
    Generator* gen = Gen_table_get_gen(gen_table, gen_index);
    if (gen != NULL && !Device_sync(
                (Device*)gen,
                Player_get_device_states(handle->player)))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing generator");
        return NULL;
    }

    return conf;
}


static Generator* add_generator(
        kqt_Handle* handle,
        Instrument* ins,
        Gen_table* gen_table,
        int gen_index)
{
    assert(handle != NULL);
    assert(ins != NULL);
    assert(gen_table != NULL);
    assert(gen_index >= 0);
    assert(gen_index < KQT_GENERATORS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new generator";

    Module* module = Handle_get_module(handle);

    // Return existing generator
    Generator* gen = Gen_table_get_gen(gen_table, gen_index);
    if (gen != NULL)
        return gen;

    // Create new generator
    gen = new_Generator(
            Instrument_get_params(ins),
            Device_get_buffer_size((Device*)module),
            Device_get_mix_rate((Device*)module));
    if (gen == NULL || !Gen_table_set_gen(gen_table, gen_index, gen))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_Generator(gen);
        return NULL;
    }

    return gen;
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
        return true;

#if 0
    if (!string_has_prefix(subkey, MAGIC_ID "gXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "g" KQT_FORMAT_VERSION "/"))
        return true;

    subkey = strchr(subkey, '/');
    ++subkey;
#endif

    Module* module = Handle_get_module(handle);

    Instrument* ins = Ins_table_get(Module_get_insts(module), ins_index);
    ins = add_instrument(handle, ins_index);
    if (ins == NULL)
        return false;

    Gen_table* table = Instrument_get_gens(ins);
    assert(table != NULL);

    if (string_eq(subkey, "p_manifest.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }

        Gen_table_set_existent(table, gen_index, existent);
    }
    else if (string_eq(subkey, "p_gen_type.json"))
    {
        if (data == NULL)
        {
            Generator* gen = Gen_table_get_gen(table, gen_index);
            if (gen != NULL)
            {
                //Connections_disconnect(module->connections,
                //                       (Device*)gen);
            }
            Gen_table_remove_gen(table, gen_index);
        }
        else
        {
            Generator* gen = add_generator(handle, ins, table, gen_index);
            if (gen == NULL)
                return false;

            // Create the Generator implementation
            Read_state* state = Read_state_init(READ_STATE_AUTO, key);
            char type[GEN_TYPE_LENGTH_MAX] = "";
            read_string(data, type, GEN_TYPE_LENGTH_MAX, state);
            if (state->error)
            {
                set_parse_error(handle, state);
                return false;
            }
            Generator_cons* cons = Gen_type_find_cons(type);
            if (cons == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_FORMAT,
                        "Unsupported Generator type: %s", type);
                return false;
            }
            Device_impl* gen_impl = cons(
                    gen,
                    Device_get_buffer_size((Device*)module),
                    Device_get_mix_rate((Device*)module));
            if (gen_impl == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory for generator implementation");
                return false;
            }

            Device_set_impl((Device*)gen, gen_impl);

            // Remove old Generator Device state
            Device_states* dstates = Player_get_device_states(handle->player);
            Device_states_remove_state(dstates, Device_get_id((Device*)gen));

            // Allocate Voice state space
            Generator_property* property = Gen_type_find_property(type);
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
//                    fprintf(stderr, "Reserving space for %" PRId64 " bytes\n",
//                                    size);
                    if (!Player_reserve_voice_state_space(
                                handle->player, size) ||
                            !Player_reserve_voice_state_space(
                                handle->length_counter, size))
                    {
                        kqt_Handle_set_error(handle, ERROR_MEMORY,
                                "Couldn't allocate memory");
                        del_Device_impl(gen_impl);
                        return false;
                    }
                }
            }

            // Allocate Device state(s) for this Generator
            Device_state* ds = Device_create_state((Device*)gen);
            if (ds == NULL || !Device_states_add_state(dstates, ds))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_Device_state(ds);
                del_Generator(gen);
                return false;
            }

            // Sync the Generator
            if (!Device_sync(
                        (Device*)gen,
                        Player_get_device_states(handle->player)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory while syncing generator");
                return false;
            }
        }
    }
    else if (string_eq(subkey, "p_events.json"))
    {
        Generator* gen = add_generator(handle, ins, table, gen_index);
        if (gen == NULL)
            return false;

        Gen_conf* conf = add_gen_conf(handle, table, gen_index);
        if (conf == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Device_params_parse_events(
                    conf->params,
                    DEVICE_EVENT_TYPE_GENERATOR,
                    handle->player,
                    data,
                    state))
        {
            set_parse_error(handle, state);
            return false;
        }
    }
    else
    {
        Generator* gen = add_generator(handle, ins, table, gen_index);
        if (gen == NULL)
            return false;

        Gen_conf* conf = add_gen_conf(handle, table, gen_index);
        if (conf == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Gen_conf_parse(conf, subkey, data, length, (Device*)gen, state))
        {
            if (!state->error)
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            else
                set_parse_error(handle, state);

            return false;
        }

        // Update Device state
        if (gen != NULL && !Device_update_state_key(
                    (Device*)gen,
                    Player_get_device_states(handle->player),
                    subkey))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
    }

    return true;
}


static Effect* add_effect(kqt_Handle* handle, int index, Effect_table* table)
{
    assert(handle != NULL);
    assert(index >= 0);
    assert(table != NULL);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new effect";

    Module* module = Handle_get_module(handle);

    // Return existing effect
    Effect* eff = Effect_table_get(table, index);
    if (eff != NULL)
        return eff;

    // Create new effect
    eff = new_Effect(
            Device_get_buffer_size((Device*)module),
            Device_get_mix_rate((Device*)module));
    if (eff == NULL || !Effect_table_set(table, index, eff))
    {
        del_Effect(eff);
        kqt_Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        return NULL;
    }

    // Allocate Device states for the new Effect
    const Device* eff_devices[] =
    {
        (Device*)eff,
        Effect_get_input_interface(eff),
        Effect_get_output_interface(eff),
        NULL
    };
    for (int i = 0; i < 3; ++i)
    {
        assert(eff_devices[i] != NULL);
        Device_state* ds = Device_create_state(eff_devices[i]);
        if (ds == NULL || !Device_states_add_state(
                    Player_get_device_states(handle->player), ds))
        {
            del_Device_state(ds);
            kqt_Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
            Effect_table_remove(table, index);
            return NULL;
        }
    }

    return eff;
}


static bool parse_effect_level(kqt_Handle* handle,
                               Instrument* ins,
                               const char* key,
                               const char* subkey,
                               void* data,
                               long length,
                               int eff_index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    int max_index = KQT_EFFECTS_MAX;
    if (ins != NULL)
    {
        max_index = KQT_INST_EFFECTS_MAX;
    }
    if (eff_index < 0 || eff_index >= max_index)
    {
        return true;
    }
#if 0
    if (!string_has_prefix(subkey, MAGIC_ID "eXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "e" KQT_FORMAT_VERSION "/"))
    {
        return true;
    }
    subkey = strchr(subkey, '/') + 1;
#endif

    Module* module = Handle_get_module(handle);

    int dsp_index = -1;
    Effect_table* table = Module_get_effects(module);
    if (ins != NULL)
    {
        table = Instrument_get_effects(ins);
    }
    if ((dsp_index = string_extract_index(subkey, "dsp_", 2, "/")) >= 0)
    {
        subkey = strchr(subkey, '/');
        assert(subkey != NULL);
        ++subkey;
        Effect* eff = Effect_table_get(table, eff_index);
        bool changed = eff != NULL && Effect_get_dsp(eff, dsp_index) != NULL;

        eff = add_effect(handle, eff_index, table);
        if (eff == NULL)
            return false;

        bool success = parse_dsp_level(handle, eff, key, subkey,
                                       data, length, dsp_index);
        changed ^= eff != NULL && Effect_get_dsp(eff, dsp_index) != NULL;
        Connections* graph = module->connections;
        if (changed && graph != NULL)
        {
            if (!prepare_connections(handle))
                return false;
        }
        return success;
    }
    else if (string_eq(subkey, "p_manifest.json"))
    {
        Effect* eff = add_effect(handle, eff_index, table);
        if (eff == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }
        Device_set_existent((Device*)eff, existent);
    }
    else if (string_eq(subkey, "p_connections.json"))
    {
        bool reconnect = false;
        Effect* eff = Effect_table_get(table, eff_index);
        if (data == NULL)
        {
            if (eff != NULL)
            {
                Effect_set_connections(eff, NULL);
                reconnect = true;
            }
        }
        else
        {
            eff = add_effect(handle, eff_index, table);
            if (eff == NULL)
                return false;

            Read_state* state = Read_state_init(READ_STATE_AUTO, key);
            Connection_level level = CONNECTION_LEVEL_EFFECT;
            if (ins != NULL)
            {
                level |= CONNECTION_LEVEL_INSTRUMENT;
            }
            Connections* graph = new_Connections_from_string(data, level,
                                                 Module_get_insts(module),
                                                 table,
                                                 Effect_get_dsps(eff),
                                                 (Device*)eff,
                                                 state);
            if (graph == NULL)
            {
                if (state->error)
                    set_parse_error(handle, state);
                else
                    kqt_Handle_set_error(handle, ERROR_MEMORY,
                            "Couldn't allocate memory");
                return false;
            }
            Effect_set_connections(eff, graph);
            reconnect = true;
        }
        if (reconnect)
        {
            Connections* global_graph = module->connections;
            if (global_graph != NULL)
            {
                if (!prepare_connections(handle))
                    return false;
            }
        }
    }
    return true;
}


static DSP_conf* add_dsp_conf(
        kqt_Handle* handle,
        DSP_table* dsp_table,
        int dsp_index)
{
    assert(handle != NULL);
    assert(dsp_table != NULL);
    assert(dsp_index >= 0);
    assert(dsp_index < KQT_DSPS_MAX);

    // Add DSP configuration
    DSP_conf* conf = DSP_table_add_conf(dsp_table, dsp_index);
    if (conf == NULL)
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory for DSP configuration");
        return NULL;
    }

    // Sync the DSP
    DSP* dsp = DSP_table_get_dsp(dsp_table, dsp_index);
    if (dsp != NULL && !Device_sync(
                (Device*)dsp,
                Player_get_device_states(handle->player)))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing DSP");
        return NULL;
    }

    return conf;
}


static DSP* add_dsp(
        kqt_Handle* handle,
        Effect* eff,
        DSP_table* dsp_table,
        int dsp_index)
{
    assert(handle != NULL);
    assert(eff != NULL);
    assert(dsp_table != NULL);
    assert(dsp_index >= 0);
    //assert(dsp_index < KQT_DSPS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new DSP";

    Module* module = Handle_get_module(handle);

    // Return existing DSP
    DSP* dsp = DSP_table_get_dsp(dsp_table, dsp_index);
    if (dsp != NULL)
        return dsp;

    // Create new DSP
    dsp = new_DSP(
            Device_get_buffer_size((Device*)module),
            Device_get_mix_rate((Device*)module));
    if (dsp == NULL || !DSP_table_set_dsp(dsp_table, dsp_index, dsp))
    {
        kqt_Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_DSP(dsp);
        return NULL;
    }

    return dsp;
}


static bool parse_dsp_level(kqt_Handle* handle,
                            Effect* eff,
                            const char* key,
                            const char* subkey,
                            void* data,
                            long length,
                            int dsp_index)
{
    assert(handle_is_valid(handle));
    assert(eff != NULL);
    assert(key != NULL);
    assert(subkey != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);

    if (dsp_index < 0 || dsp_index >= KQT_DSPS_MAX)
        return true;

#if 0
    if (!string_has_prefix(subkey, MAGIC_ID "dXX/") &&
            !string_has_prefix(subkey, MAGIC_ID "d" KQT_FORMAT_VERSION "/"))
        return true;

    subkey = strchr(subkey, '/') + 1;
#endif

    Module* module = Handle_get_module(handle);

    DSP_table* dsp_table = Effect_get_dsps(eff);
    assert(dsp_table != NULL);

    if (string_eq(subkey, "p_manifest.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }

        DSP_table_set_existent(dsp_table, dsp_index, existent);
    }
    else if (string_eq(subkey, "p_dsp_type.json"))
    {
//        fprintf(stderr, "%s\n", subkey);
        if (data == NULL)
        {
            DSP_table_remove_dsp(dsp_table, dsp_index);
        }
        else
        {
            DSP* dsp = add_dsp(handle, eff, dsp_table, dsp_index);
            if (dsp == NULL)
                return false;

            // Create the DSP implementation
            Read_state* state = Read_state_init(READ_STATE_AUTO, key);
            char type[DSP_TYPE_LENGTH_MAX] = "";
            read_string(data, type, DSP_TYPE_LENGTH_MAX, state);
            if (state->error)
            {
                set_parse_error(handle, state);
                return false;
            }
            DSP_cons* cons = DSP_type_find_cons(type);
            if (cons == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_FORMAT,
                        "Unsupported DSP type: %s", type);
                return false;
            }
            Device_impl* dsp_impl = cons(
                    dsp,
                    Device_get_buffer_size((Device*)module),
                    Device_get_mix_rate((Device*)module));
            if (dsp_impl == NULL)
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory for DSP implementation");
                return false;
            }

            Device_set_impl((Device*)dsp, dsp_impl);

            // Remove old DSP Device state
            Device_states* dstates = Player_get_device_states(handle->player);
            Device_states_remove_state(dstates, Device_get_id((Device*)dsp));

            // Allocate Device state(s) for this DSP
            Device_state* ds = Device_create_state((Device*)dsp);
            if (ds == NULL || !Device_states_add_state(
                        Player_get_device_states(handle->player), ds))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_Device_state(ds);
                del_DSP(dsp);
                return false;
            }

            // Set DSP resources
            if (!Device_set_mix_rate((Device*)dsp,
                        dstates,
                        Player_get_audio_rate(handle->player)) ||
                    !Device_set_buffer_size((Device*)dsp,
                        dstates,
                        Player_get_audio_buffer_size(handle->player)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory for DSP state");
                return false;
            }

            // Sync the DSP
            if (!Device_sync(
                        (Device*)dsp,
                        Player_get_device_states(handle->player)))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory while syncing DSP");
                return false;
            }
        }
    }
    else if ((string_has_prefix(subkey, "i/") ||
              string_has_prefix(subkey, "c/")) &&
             key_is_device_param(subkey))
    {
        DSP* dsp = add_dsp(handle, eff, dsp_table, dsp_index);
        if (dsp == NULL)
            return false;

        DSP_conf* conf = add_dsp_conf(handle, dsp_table, dsp_index);
        if (conf == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!DSP_conf_parse(conf, subkey, data, length, (Device*)dsp, state))
        {
            if (!state->error)
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            else
                set_parse_error(handle, state);
            return false;
        }

        // Update Device state
        if (dsp != NULL && !Device_update_state_key(
                    (Device*)dsp,
                    Player_get_device_states(handle->player),
                    subkey))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            return false;
        }
    }
    else if (string_eq(subkey, "p_events.json"))
    {
        DSP* dsp = add_dsp(handle, eff, dsp_table, dsp_index);
        if (dsp == NULL)
            return false;

        DSP_conf* conf = add_dsp_conf(handle, dsp_table, dsp_index);
        if (conf == NULL)
            return false;

        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        if (!Device_params_parse_events(
                    conf->params,
                    DEVICE_EVENT_TYPE_DSP,
                    handle->player,
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
    if (index < 0 || index >= KQT_PATTERNS_MAX)
    {
        return true;
    }

    Module* module = Handle_get_module(handle);

    if (string_eq(subkey, "p_manifest.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }
        Pat_table* pats = Module_get_pats(module);
        Pat_table_set_existent(pats, index, existent);
    }
    else if (string_eq(subkey, "p_pattern.json"))
    {
        Pattern* pat = Pat_table_get(Module_get_pats(module), index);
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
        if (new_pattern && !Pat_table_set(Module_get_pats(module), index, pat))
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
    int sub_index = 0;
    ++second_element;
    if ((sub_index = string_extract_index(subkey, "col_", 2, "/")) >= 0 &&
                string_eq(second_element, "p_triggers.json"))
    {
        if (sub_index >= KQT_COLUMNS_MAX)
        {
            return true;
        }
        Pattern* pat = Pat_table_get(Module_get_pats(module), index);
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
        AAiter* locations_iter = NULL;
        AAtree* locations = Pattern_get_locations(pat, &locations_iter);
        const Event_names* event_names =
                Event_handler_get_names(Player_get_event_handler(handle->player));
        Column* col = new_Column_from_string(Pattern_get_length(pat),
                                             data,
                                             locations,
                                             locations_iter,
                                             event_names,
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
        if (!Pattern_set_column(pat, sub_index, col))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            if (new_pattern)
            {
                del_Pattern(pat);
            }
            return false;
        }
        if (new_pattern)
        {
            if (!Pat_table_set(Module_get_pats(module), index, pat))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_Pattern(pat);
                return false;
            }
        }
    }
    else if ((sub_index = string_extract_index(subkey, "instance_", 3, "/")) >= 0)
    {
        Pattern* pat = Pat_table_get(Module_get_pats(module), index);
        bool new_pattern = (pat == NULL);
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

        assert(pat != NULL);
        if (!parse_pat_inst_level(
                    handle,
                    pat,
                    key,
                    second_element,
                    data,
                    length,
                    sub_index))
        {
            if (new_pattern)
            {
                del_Pattern(pat);
            }
            return false;
        }

        if (new_pattern && !Pat_table_set(Module_get_pats(module), index, pat))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            del_Pattern(pat);
            return false;
        }
        return true;
    }
    return true;
}


static bool parse_pat_inst_level(
        kqt_Handle* handle,
        Pattern* pat,
        const char* key,
        const char* subkey,
        void* data,
        long length,
        int index)
{
    assert(handle_is_valid(handle));
    assert(key != NULL);
    assert(subkey != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    (void)length;

    if (string_eq(subkey, "p_manifest.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }

        Pattern_set_inst_existent(pat, index, existent);
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

    Module* module = Handle_get_module(handle);

    if (string_eq(subkey, "p_scale.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Scale* scale = new_Scale_from_string(data, state);
        if (scale == NULL)
        {
            set_parse_error(handle, state);
            return false;
        }
        Module_set_scale(module, index, scale);
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
    if (index < 0 || index >= KQT_SONGS_MAX)
    {
        return true;
    }

    Module* module = Handle_get_module(handle);

    if (string_eq(subkey, "p_manifest.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        const bool existent = read_default_manifest(data, state);
        if (state->error)
        {
            set_parse_error(handle, state);
            return false;
        }
        Song_table_set_existent(module->songs, index, existent);
    }
    else if (string_eq(subkey, "p_song.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Song* song = new_Song_from_string(data, state);
        if (song == NULL)
        {
            if (!state->error)
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            else
                set_parse_error(handle, state);

            return false;
        }
        Song_table* st = Module_get_songs(module);
        if (!Song_table_set(st, index, song))
        {
            kqt_Handle_set_error(handle, ERROR_MEMORY,
                    "Couldn't allocate memory");
            del_Song(song);
            return false;
        }
    }
    else if (string_eq(subkey, "p_order_list.json"))
    {
        Read_state* state = Read_state_init(READ_STATE_AUTO, key);
        Order_list* ol = new_Order_list(data, state);
        if (ol == NULL)
        {
            if (!state->error)
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
            else
                set_parse_error(handle, state);

            return false;
        }

        // Update pattern location information
        // This is required for correct update of jump counters.
        const size_t ol_len = Order_list_get_len(ol);
        for (size_t i = 0; i < ol_len; ++i)
        {
            Pat_inst_ref* ref = Order_list_get_pat_inst_ref(ol, i);
            int16_t pat_index = ref->pat;
            Pat_table* pats = Module_get_pats(module);
            assert(pats != NULL);
            Pattern* pat = Pat_table_get(pats, pat_index);
            if (pat == NULL)
                continue;

            if (!Pattern_set_location(pat, index, ref))
            {
                kqt_Handle_set_error(handle, ERROR_MEMORY,
                        "Couldn't allocate memory");
                del_Order_list(ol);
                return false;
            }
        }

        if (module->order_lists[index] != NULL)
            del_Order_list(module->order_lists[index]);

        module->order_lists[index] = ol;
    }
    return true;
}


