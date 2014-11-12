

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
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include <Connections.h>
#include <debug/assert.h>
#include <devices/Device_event_keys.h>
#include <devices/Device_params.h>
#include <devices/dsps/DSP_type.h>
#include <devices/generators/Gen_type.h>
#include <Handle_private.h>
#include <memory.h>
#include <module/Bind.h>
#include <module/Environment.h>
#include <module/manifest.h>
#include <module/Parse_manager.h>
#include <string/common.h>
#include <string/key_pattern.h>
#include <string/Streader.h>


typedef struct Reader_params
{
    Handle* handle;
    const int32_t* indices;
    const char* subkey;
    Streader* sr;
} Reader_params;


#define MODULE_KEYP(name, keyp, def) static bool read_##name(Reader_params* params);
#include <module/Module_key_patterns.h>


static const struct
{
    const char* keyp;
    bool (*func)(Reader_params*);
} keyp_to_func[] =
{
#define MODULE_KEYP(name, keyp, def) { keyp, read_##name, },
#include <module/Module_key_patterns.h>
    { NULL, NULL }
};


#define set_error(params)                                                        \
    if (true)                                                                    \
    {                                                                            \
        if (Error_get_type(&(params)->sr->error) == ERROR_FORMAT)                \
            Handle_set_validation_error_from_Error(                              \
                    (params)->handle, &(params)->sr->error);                     \
        else                                                                     \
            Handle_set_error_from_Error((params)->handle, &(params)->sr->error); \
    } else (void)0


static bool is_ins_out_conn_possible(Handle* handle, int32_t ins_index, int32_t port)
{
    assert(handle != NULL);
    const Module* module = Handle_get_module(handle);

    const Instrument* ins = Ins_table_get(Module_get_insts(module), ins_index);
    if (ins == NULL)
        return false;

    return Device_get_port_existence((const Device*)ins, DEVICE_PORT_TYPE_SEND, port);
}


static bool is_gen_out_conn_possible(
        Handle* handle, int32_t ins_index, int32_t gen_index, int32_t port)
{
    assert(handle != NULL);

    const Module* module = Handle_get_module(handle);

    const Instrument* ins = Ins_table_get(Module_get_insts(module), ins_index);
    if (ins == NULL)
        return false;

    const Generator* gen = Instrument_get_gen(ins, gen_index);
    if ((gen == NULL) || !Device_has_complete_type((const Device*)gen))
        return false;

    return Device_get_port_existence((const Device*)gen, DEVICE_PORT_TYPE_SEND, port);
}


static const Effect_table* find_effect_table(Handle* handle, int32_t ins_index)
{
    assert(handle != NULL);

    const Module* module = Handle_get_module(handle);

    const Effect_table* eff_table = Module_get_effects(module);
    if (ins_index >= 0)
    {
        Instrument* ins = Ins_table_get(Module_get_insts(module), ins_index);
        if (ins == NULL)
            return NULL;

        eff_table = Instrument_get_effects(ins);
    }

    return eff_table;
}


static const Effect* find_effect(Handle* handle, int32_t ins_index, int32_t eff_index)
{
    assert(handle != NULL);

    const Effect_table* eff_table = find_effect_table(handle, ins_index);
    if (eff_table == NULL)
        return NULL;

    return Effect_table_get(eff_table, eff_index);
}


static bool is_eff_in_conn_possible(
        Handle* handle, int32_t ins_index, int32_t eff_index, int32_t port)
{
    assert(handle != NULL);

    const Effect* eff = find_effect(handle, ins_index, eff_index);
    if (eff == NULL)
        return false;

    return Device_get_port_existence((const Device*)eff, DEVICE_PORT_TYPE_RECEIVE, port);
}


static bool is_eff_out_conn_possible(
        Handle* handle, int32_t ins_index, int32_t eff_index, int32_t port)
{
    assert(handle != NULL);

    const Effect* eff = find_effect(handle, ins_index, eff_index);
    if (eff == NULL)
        return false;

    return Device_get_port_existence((const Device*)eff, DEVICE_PORT_TYPE_SEND, port);
}


static const DSP* find_complete_dsp(
        Handle* handle, int32_t ins_index, int32_t eff_index, int32_t dsp_index)
{
    assert(handle != NULL);

    const Effect* eff = find_effect(handle, ins_index, eff_index);
    if (eff == NULL)
        return NULL;

    const DSP* dsp = Effect_get_dsp(eff, dsp_index);
    if ((dsp == NULL) || !Device_has_complete_type((const Device*)dsp))
        return NULL;

    return dsp;
}


static bool is_dsp_in_conn_possible(
        Handle* handle,
        int32_t ins_index,
        int32_t eff_index,
        int32_t dsp_index,
        int32_t port)
{
    assert(handle != NULL);

    const DSP* dsp = find_complete_dsp(handle, ins_index, eff_index, dsp_index);
    if (dsp == NULL)
        return false;

    return Device_get_port_existence((const Device*)dsp, DEVICE_PORT_TYPE_RECEIVE, port);
}


static bool is_dsp_out_conn_possible(
        Handle* handle,
        int32_t ins_index,
        int32_t eff_index,
        int32_t dsp_index,
        int32_t port)
{
    assert(handle != NULL);

    const DSP* dsp = find_complete_dsp(handle, ins_index, eff_index, dsp_index);
    if (dsp == NULL)
        return false;

    return Device_get_port_existence((const Device*)dsp, DEVICE_PORT_TYPE_SEND, port);
}


static bool is_connection_possible(
        Handle* handle, const char* keyp, const Key_indices indices)
{
    assert(handle != NULL);
    assert(keyp != NULL);
    assert(indices != NULL);

    if (string_has_prefix(keyp, "ins_XX/eff_XX/dsp_XX/in_XX/"))
        return is_dsp_in_conn_possible(
                handle, indices[0], indices[1], indices[2], indices[3]);
    else if (string_has_prefix(keyp, "ins_XX/eff_XX/dsp_XX/out_XX/"))
        return is_dsp_out_conn_possible(
                handle, indices[0], indices[1], indices[2], indices[3]);
    else if (string_has_prefix(keyp, "ins_XX/eff_XX/in_XX/"))
        return is_eff_in_conn_possible(handle, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "ins_XX/eff_XX/out_XX/"))
        return is_eff_out_conn_possible(handle, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "ins_XX/gen_XX/out_XX/"))
        return is_gen_out_conn_possible(handle, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "ins_XX/out_XX/"))
        return is_ins_out_conn_possible(handle, indices[0], indices[1]);
    else if (string_has_prefix(keyp, "eff_XX/dsp_XX/in_XX/"))
        return is_dsp_in_conn_possible(handle, -1, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "eff_XX/dsp_XX/out_XX/"))
        return is_dsp_out_conn_possible(handle, -1, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "eff_XX/in_XX/"))
        return is_eff_in_conn_possible(handle, -1, indices[0], indices[1]);
    else if (string_has_prefix(keyp, "eff_XX/out_XX/"))
        return is_eff_out_conn_possible(handle, -1, indices[0], indices[1]);

    return false;
}


bool parse_data(
        Handle* handle,
        const char* key,
        const void* data,
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

    // Get key pattern info
    char key_pattern[KQT_KEY_LENGTH_MAX] = "";
    Key_indices key_indices = { 0 };
    for (int i = 0; i < KEY_INDICES_MAX; ++i)
        key_indices[i] = -1;

    if (!extract_key_pattern(key, key_pattern, key_indices))
    {
        fprintf(stderr, "invalid key: %s\n", key);
        assert(false);
        return false;
    }

    assert(strlen(key) == strlen(key_pattern));

    const bool was_connection_possible = is_connection_possible(
            handle, key_pattern, key_indices);

    // Find a known key pattern that is a prefix of our retrieved key pattern
    for (int i = 0; keyp_to_func[i].keyp != NULL; ++i)
    {
        if (string_has_prefix(key_pattern, keyp_to_func[i].keyp))
        {
            // Fill in params and send them to our callback
            Reader_params params;
            params.handle = handle;
            params.indices = key_indices;
            params.subkey = key + strlen(keyp_to_func[i].keyp);
            params.sr = Streader_init(STREADER_AUTO, data, length);

            const bool success = keyp_to_func[i].func(&params);
            if (!success)
                return false;

            // Mark connections for update if needed
            if (was_connection_possible != is_connection_possible(
                        handle, key_pattern, key_indices))
                handle->update_connections = true;

            return true;
        }
    }

    // Accept unknown key pattern without modification
    return true;
}


static bool read_composition(Reader_params* params)
{
    assert(params != NULL);

    if (!Module_parse_composition(Handle_get_module(params->handle), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_connections(Reader_params* params)
{
    assert(params != NULL);

    Module* module = Handle_get_module(params->handle);

    Connections* graph = new_Connections_from_string(
            params->sr,
            CONNECTION_LEVEL_GLOBAL,
            Module_get_insts(module),
            Module_get_effects(module),
            NULL,
            (Device*)module);
    if (graph == NULL)
    {
        set_error(params);
        return false;
    }

    if (module->connections != NULL)
        del_Connections(module->connections);

    module->connections = graph;

    params->handle->update_connections = true;

    return true;
}


static bool read_control_map(Reader_params* params)
{
    assert(params != NULL);

    if (!Module_set_ins_map(Handle_get_module(params->handle), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_control_manifest(Reader_params* params)
{
    assert(params != NULL);

    const int32_t index = params->indices[0];
    if (index < 0 || index >= KQT_CONTROLS_MAX)
        return true;

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Module_set_control(Handle_get_module(params->handle), index, existent);

    return true;
}


static bool read_random_seed(Reader_params* params)
{
    assert(params != NULL);

    if (!Module_parse_random_seed(Handle_get_module(params->handle), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_environment(Reader_params* params)
{
    assert(params != NULL);

    if (!Environment_parse(Handle_get_module(params->handle)->env, params->sr))
    {
        set_error(params);
        return false;
    }

    if (!Player_refresh_env_state(params->handle->player))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for environment state");
        return false;
    }

    return true;
}


static bool read_bind(Reader_params* params)
{
    assert(params != NULL);

    Bind* map = new_Bind(
            params->sr,
            Event_handler_get_names(Player_get_event_handler(params->handle->player)));
    if (map == NULL)
    {
        set_error(params);
        return false;
    }

    Module_set_bind(Handle_get_module(params->handle), map);

    if (!Player_refresh_bind_state(params->handle->player))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for bind state");
        return false;
    }

    return true;
}


static bool read_album_manifest(Reader_params* params)
{
    assert(params != NULL);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Handle_get_module(params->handle)->album_is_existent = existent;

    return true;
}


static bool read_album_tracks(Reader_params* params)
{
    assert(params != NULL);

    Track_list* tl = new_Track_list(params->sr);
    if (tl == NULL)
    {
        set_error(params);
        return false;
    }

    Module* module = Handle_get_module(params->handle);

    del_Track_list(module->track_list);
    module->track_list = tl;

    return true;
}


static Instrument* add_instrument(Handle* handle, int index)
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
    ins = new_Instrument();
    if (ins == NULL || !Ins_table_set(Module_get_insts(module), index, ins))
    {
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_Instrument(ins);
        return NULL;
    }

    // Allocate Device state(s) for the new Instrument
    Device_state* ds = Device_create_state(
            (Device*)ins,
            Player_get_audio_rate(handle->player),
            Player_get_audio_buffer_size(handle->player));
    if (ds == NULL || !Device_states_add_state(
                Player_get_device_states(handle->player), ds))
    {
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        Ins_table_remove(Module_get_insts(module), index);
        return NULL;
    }

    return ins;
}


#define acquire_port_index(index, params, depth)            \
    if (true)                                               \
    {                                                       \
        (index) = (params)->indices[(depth)];               \
        if ((index) < 0 || (index) >= KQT_DEVICE_PORTS_MAX) \
            return true;                                    \
    }                                                       \
    else (void)0


#define acquire_ins_index(index, params)                   \
    if (true)                                              \
    {                                                      \
        (index) = (params)->indices[0];                    \
        if ((index) < 0 || (index) >= KQT_INSTRUMENTS_MAX) \
            return true;                                   \
    }                                                      \
    else (void)0


#define acquire_ins(ins, handle, index)            \
    if (true)                                      \
    {                                              \
        (ins) = add_instrument((handle), (index)); \
        if ((ins) == NULL)                         \
            return false;                          \
    }                                              \
    else (void)0


static bool read_ins_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_ins_index(index, params);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, index);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Device_set_existent((Device*)ins, existent);

    return true;
}


static bool read_ins(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_ins_index(index, params);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, index);

    if (!Instrument_parse_header(ins, params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_ins_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t ins_index = -1;
    acquire_ins_index(ins_index, params);
    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, 1);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, ins_index);

    Device_set_port_existence(
            (Device*)ins, DEVICE_PORT_TYPE_RECEIVE, out_port_index, existent);
    Device_set_port_existence(
            (Device*)ins, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


static bool read_ins_connections(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_ins_index(index, params);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, index);

    if (!Streader_has_data(params->sr))
    {
        Instrument_set_connections(ins, NULL);
        params->handle->update_connections = true;
    }
    else
    {
        Connections* graph = new_Connections_from_string(
                params->sr,
                CONNECTION_LEVEL_INSTRUMENT,
                Module_get_insts(Handle_get_module(params->handle)),
                Instrument_get_effects(ins),
                NULL,
                (Device*)ins);
        if (graph == NULL)
        {
            set_error(params);
            return false;
        }

        Instrument_set_connections(ins, graph);
        params->handle->update_connections = true;
    }

    return true;
}


static bool read_ins_env_generic(
        Reader_params* params, bool parse_func(Instrument_params*, Streader*))
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_ins_index(index, params);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, index);

    if (!parse_func(Instrument_get_params(ins), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_ins_env_force(Reader_params* params)
{
    assert(params != NULL);
    return read_ins_env_generic(params, Instrument_params_parse_env_force);
}


static bool read_ins_env_force_release(Reader_params* params)
{
    assert(params != NULL);
    return read_ins_env_generic(params, Instrument_params_parse_env_force_rel);
}


static bool read_ins_env_force_filter(Reader_params* params)
{
    assert(params != NULL);
    return read_ins_env_generic(params, Instrument_params_parse_env_force_filter);
}


static bool read_ins_env_pitch_pan(Reader_params* params)
{
    assert(params != NULL);
    return read_ins_env_generic(params, Instrument_params_parse_env_pitch_pan);
}


static Generator* add_generator(
        Handle* handle,
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

    // Return existing generator
    Generator* gen = Gen_table_get_gen_mut(gen_table, gen_index);
    if (gen != NULL)
        return gen;

    // Create new generator
    gen = new_Generator(Instrument_get_params(ins));
    if (gen == NULL || !Gen_table_set_gen(gen_table, gen_index, gen))
    {
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_Generator(gen);
        return NULL;
    }

    return gen;
}


#define acquire_gen_index(index, params)                  \
    if (true)                                             \
    {                                                     \
        (index) = (params)->indices[1];                   \
        if ((index) < 0 || (index) >= KQT_GENERATORS_MAX) \
            return true;                                  \
    }                                                     \
    else (void)0


static bool read_gen_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t ins_index = -1;
    acquire_ins_index(ins_index, params);
    int32_t gen_index = -1;
    acquire_gen_index(gen_index, params);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, ins_index);

    Gen_table* table = Instrument_get_gens(ins);
    assert(table != NULL);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Gen_table_set_existent(table, gen_index, existent);

    return true;
}


static bool read_gen_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t ins_index = -1;
    acquire_ins_index(ins_index, params);
    int32_t gen_index = -1;
    acquire_gen_index(gen_index, params);
    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, 2);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, ins_index);
    Gen_table* gen_table = Instrument_get_gens(ins);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Generator* gen = add_generator(params->handle, ins, gen_table, gen_index);
    if (gen == NULL)
        return false;

    Device_set_port_existence(
            (Device*)gen, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


static bool read_gen_type(Reader_params* params)
{
    assert(params != NULL);

    int32_t ins_index = -1;
    acquire_ins_index(ins_index, params);
    int32_t gen_index = -1;
    acquire_gen_index(gen_index, params);

    if (!Streader_has_data(params->sr))
    {
        // Remove generator
        Module* module = Handle_get_module(params->handle);
        Instrument* ins = Ins_table_get(Module_get_insts(module), ins_index);
        if (ins == NULL)
            return true;

        Gen_table* gen_table = Instrument_get_gens(ins);
        Gen_table_remove_gen(gen_table, gen_index);

        return true;
    }

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, ins_index);
    Gen_table* gen_table = Instrument_get_gens(ins);

    Generator* gen = add_generator(params->handle, ins, gen_table, gen_index);
    if (gen == NULL)
        return false;

    // Create the Generator implementation
    char type[GEN_TYPE_LENGTH_MAX] = "";
    if (!Streader_read_string(params->sr, GEN_TYPE_LENGTH_MAX, type))
    {
        set_error(params);
        return false;
    }

    Generator_cons* cons = Gen_type_find_cons(type);
    if (cons == NULL)
    {
        Handle_set_error(params->handle, ERROR_FORMAT,
                "Unsupported Generator type: %s", type);
        return false;
    }

    Device_impl* gen_impl = cons(gen);
    if (gen_impl == NULL)
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for generator implementation");
        return false;
    }

    if (!Device_set_impl((Device*)gen, gen_impl))
    {
        del_Device_impl(gen_impl);
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while initialising Generator implementation");
        return false;
    }

    // Remove old Generator Device state
    Device_states* dstates = Player_get_device_states(params->handle->player);
    Device_states_remove_state(dstates, Device_get_id((Device*)gen));

    // Get generator properties
    Generator_property* property = Gen_type_find_property(type);
    if (property != NULL)
    {
        // Allocate Voice state space
        const char* size_str = property(gen, "voice_state_size");
        if (size_str != NULL)
        {
            Streader* size_sr = Streader_init(
                    STREADER_AUTO, size_str, strlen(size_str));
            int64_t size = 0;
            Streader_read_int(size_sr, &size);
            assert(!Streader_is_error_set(params->sr));
            assert(size >= 0);
//            fprintf(stderr, "Reserving space for %" PRId64 " bytes\n",
//                            size);
            if (!Player_reserve_voice_state_space(
                        params->handle->player, size) ||
                    !Player_reserve_voice_state_space(
                        params->handle->length_counter, size))
            {
                Handle_set_error(params->handle, ERROR_MEMORY,
                        "Couldn't allocate memory for generator voice states");
                del_Device_impl(gen_impl);
                return false;
            }
        }

        // Allocate channel-specific generator state space
        const char* gen_state_vars = property(gen, "gen_state_vars");
        if (gen_state_vars != NULL)
        {
            Streader* gsv_sr = Streader_init(
                    STREADER_AUTO,
                    gen_state_vars,
                    strlen(gen_state_vars));

            if (!Player_alloc_channel_gen_state_keys(
                        params->handle->player, gsv_sr))
            {
                Reader_params gsv_params;
                gsv_params.handle = params->handle;
                gsv_params.sr = gsv_sr;
                set_error(&gsv_params);
                return false;
            }
        }
    }

    // Allocate Device state(s) for this Generator
    Device_state* ds = Device_create_state(
            (Device*)gen,
            Player_get_audio_rate(params->handle->player),
            Player_get_audio_buffer_size(params->handle->player));
    if (ds == NULL || !Device_states_add_state(dstates, ds))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for device state");
        del_Device_state(ds);
        del_Generator(gen);
        return false;
    }

    // Sync the Generator
    if (!Device_sync((Device*)gen))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing generator");
        return false;
    }

    // Sync the Device state(s)
    if (!Device_sync_states(
                (Device*)gen,
                Player_get_device_states(params->handle->player)))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing generator");
        return false;
    }

    return true;
}


static bool read_gen_impl_conf_key(Reader_params* params)
{
    assert(params != NULL);

    if (!key_is_device_param(params->subkey))
        return true;

    int32_t ins_index = -1;
    acquire_ins_index(ins_index, params);
    int32_t gen_index = -1;
    acquire_gen_index(gen_index, params);

    Instrument* ins = NULL;
    acquire_ins(ins, params->handle, ins_index);
    Gen_table* gen_table = Instrument_get_gens(ins);

    Generator* gen = add_generator(params->handle, ins, gen_table, gen_index);
    if (gen == NULL)
        return false;

    // Update Device
    if (!Device_set_key((Device*)gen, params->subkey, params->sr))
    {
        set_error(params);
        return false;
    }

    // Update Device state
    Device_set_state_key(
            (Device*)gen,
            Player_get_device_states(params->handle->player),
            params->subkey);

    return true;
}


static bool read_gen_impl_key(Reader_params* params)
{
    assert(params != NULL);
    assert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "i/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_gen_impl_conf_key(&hack_params);
}


static bool read_gen_conf_key(Reader_params* params)
{
    assert(params != NULL);
    assert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "c/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_gen_impl_conf_key(&hack_params);
}


static Effect* add_effect(Handle* handle, int index, Effect_table* table)
{
    assert(handle != NULL);
    assert(index >= 0);
    assert(table != NULL);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new effect";

    // Return existing effect
    Effect* eff = Effect_table_get_mut(table, index);
    if (eff != NULL)
        return eff;

    // Create new effect
    eff = new_Effect();
    if (eff == NULL || !Effect_table_set(table, index, eff))
    {
        del_Effect(eff);
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
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
        Device_state* ds = Device_create_state(
                eff_devices[i],
                Player_get_audio_rate(handle->player),
                Player_get_audio_buffer_size(handle->player));
        if (ds == NULL || !Device_states_add_state(
                    Player_get_device_states(handle->player), ds))
        {
            del_Device_state(ds);
            Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
            Effect_table_remove(table, index);
            return NULL;
        }
    }

    return eff;
}


#define acquire_effect(effect, handle, table, index)       \
    if (true)                                              \
    {                                                      \
        (effect) = add_effect((handle), (index), (table)); \
        if ((effect) == NULL)                              \
            return false;                                  \
    }                                                      \
    else (void)0


static int get_effect_index_stop(bool is_instrument)
{
    return (is_instrument ? KQT_INST_EFFECTS_MAX : KQT_EFFECTS_MAX);
}


static int get_effect_index_loc(bool is_instrument)
{
    return (is_instrument ? 1 : 0);
}


static int get_dsp_index_loc(bool is_instrument)
{
    return get_effect_index_loc(is_instrument) + 1;
}


#define acquire_effect_index(index, params, is_instrument)                  \
    if (true)                                                               \
    {                                                                       \
        const int index_stop = get_effect_index_stop((is_instrument));      \
        (index) = (params)->indices[get_effect_index_loc((is_instrument))]; \
        if ((index) < 0 || (index) >= (index_stop))                         \
            return true;                                                    \
    }                                                                       \
    else (void)0


static bool read_effect_effect_manifest(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Device_set_existent((Device*)effect, existent);

    return true;
}


static bool read_effect_effect_in_port_manifest(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t in_port_index = -1;
    acquire_port_index(in_port_index, params, is_instrument ? 2 : 1);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);

    Effect_set_port_existence(effect, DEVICE_PORT_TYPE_RECEIVE, in_port_index, existent);

    return true;
}


static bool read_effect_effect_out_port_manifest(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, is_instrument ? 2 : 1);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);

    Effect_set_port_existence(effect, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


static bool read_effect_effect_connections(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);

    if (!Streader_has_data(params->sr))
    {
        Effect_set_connections(effect, NULL);
        params->handle->update_connections = true;
    }
    else
    {
        Module* module = Handle_get_module(params->handle);

        Connection_level level = CONNECTION_LEVEL_EFFECT;
        if (eff_table != Module_get_effects(module))
            level |= CONNECTION_LEVEL_INSTRUMENT;

        Connections* graph = new_Connections_from_string(
                params->sr,
                level,
                Module_get_insts(module),
                eff_table,
                Effect_get_dsps(effect),
                (Device*)effect);
        if (graph == NULL)
        {
            set_error(params);
            return false;
        }

        Effect_set_connections(effect, graph);
        params->handle->update_connections = true;
    }

    return true;
}


static DSP* add_dsp(
        Handle* handle,
        DSP_table* dsp_table,
        int dsp_index)
{
    assert(handle != NULL);
    assert(dsp_table != NULL);
    assert(dsp_index >= 0);
    //assert(dsp_index < KQT_DSPS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new DSP";

    // Return existing DSP
    DSP* dsp = DSP_table_get_dsp(dsp_table, dsp_index);
    if (dsp != NULL)
        return dsp;

    // Create new DSP
    dsp = new_DSP();
    if (dsp == NULL || !DSP_table_set_dsp(dsp_table, dsp_index, dsp))
    {
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_DSP(dsp);
        return NULL;
    }

    return dsp;
}


#define acquire_dsp_index(index, params, is_instrument)                  \
    if (true)                                                            \
    {                                                                    \
        (index) = (params)->indices[get_dsp_index_loc((is_instrument))]; \
        if ((index) < 0 || (index) >= KQT_DSPS_MAX)                      \
            return true;                                                 \
    }                                                                    \
    else (void)0


static bool read_effect_dsp_manifest(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t dsp_index = -1;
    acquire_dsp_index(dsp_index, params, is_instrument);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Effect* effect = NULL;
    if (existent)
    {
        acquire_effect(effect, params->handle, eff_table, eff_index);
    }
    else
    {
        effect = Effect_table_get_mut(eff_table, eff_index);
        if (effect == NULL)
            return true;
    }

    DSP_table* dsp_table = Effect_get_dsps_mut(effect);
    DSP_table_set_existent(dsp_table, dsp_index, existent);

    return true;
}


static bool read_effect_dsp_in_port_manifest(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t dsp_index = -1;
    acquire_dsp_index(dsp_index, params, is_instrument);
    int32_t in_port_index = -1;
    acquire_port_index(in_port_index, params, is_instrument ? 3 : 2);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);
    DSP_table* dsp_table = Effect_get_dsps_mut(effect);

    DSP* dsp = add_dsp(params->handle, dsp_table, dsp_index);
    if (dsp == NULL)
        return false;

    Device_set_port_existence(
            (Device*)dsp, DEVICE_PORT_TYPE_RECEIVE, in_port_index, existent);

    return true;
}


static bool read_effect_dsp_out_port_manifest(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t dsp_index = -1;
    acquire_dsp_index(dsp_index, params, is_instrument);
    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, is_instrument ? 3 : 2);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);
    DSP_table* dsp_table = Effect_get_dsps_mut(effect);

    DSP* dsp = add_dsp(params->handle, dsp_table, dsp_index);
    if (dsp == NULL)
        return false;

    Device_set_port_existence(
            (Device*)dsp, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


static bool read_effect_dsp_type(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t dsp_index = -1;
    acquire_dsp_index(dsp_index, params, is_instrument);

    if (!Streader_has_data(params->sr))
    {
        // Remove DSP
        Effect* effect = Effect_table_get_mut(eff_table, eff_index);
        if (effect == NULL)
            return true;

        DSP_table* dsp_table = Effect_get_dsps_mut(effect);
        DSP_table_remove_dsp(dsp_table, dsp_index);

        return true;
    }

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);
    DSP_table* dsp_table = Effect_get_dsps_mut(effect);

    DSP* dsp = add_dsp(params->handle, dsp_table, dsp_index);
    if (dsp == NULL)
        return false;

    // Create the DSP implementation
    char type[DSP_TYPE_LENGTH_MAX] = "";
    if (!Streader_read_string(params->sr, DSP_TYPE_LENGTH_MAX, type))
    {
        set_error(params);
        return false;
    }
    DSP_cons* cons = DSP_type_find_cons(type);
    if (cons == NULL)
    {
        Handle_set_error(params->handle, ERROR_FORMAT,
                "Unsupported DSP type: %s", type);
        return false;
    }
    Device_impl* dsp_impl = cons(dsp);
    if (dsp_impl == NULL)
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for DSP implementation");
        return false;
    }

    if (!Device_set_impl((Device*)dsp, dsp_impl))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while initialising DSP implementation");
        return false;
    }

    const Player* player = params->handle->player;

    // Remove old DSP Device state
    Device_states* dstates = Player_get_device_states(player);
    Device_states_remove_state(dstates, Device_get_id((Device*)dsp));

    // Allocate Device state(s) for this DSP
    Device_state* ds = Device_create_state(
            (Device*)dsp,
            Player_get_audio_rate(player),
            Player_get_audio_buffer_size(player));
    if (ds == NULL || !Device_states_add_state(
                Player_get_device_states(player), ds))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for device state");
        del_Device_state(ds);
        del_DSP(dsp);
        return false;
    }

    // Set DSP resources
    if (!Device_set_audio_rate((Device*)dsp,
                dstates,
                Player_get_audio_rate(player)) ||
            !Device_set_buffer_size((Device*)dsp,
                dstates,
                Player_get_audio_buffer_size(player)))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for DSP state");
        return false;
    }

    // Sync the DSP
    if (!Device_sync((Device*)dsp))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing DSP");
        return false;
    }

    // Sync the Device state(s)
    if (!Device_sync_states((Device*)dsp, Player_get_device_states(player)))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing DSP");
        return false;
    }

    return true;
}


static bool read_effect_dsp_impl_conf_key(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);

    if (!key_is_device_param(params->subkey))
        return true;

    int32_t eff_index = -1;
    acquire_effect_index(eff_index, params, is_instrument);
    int32_t dsp_index = -1;
    acquire_dsp_index(dsp_index, params, is_instrument);

    Effect* effect = NULL;
    acquire_effect(effect, params->handle, eff_table, eff_index);
    DSP_table* dsp_table = Effect_get_dsps_mut(effect);

    DSP* dsp = add_dsp(params->handle, dsp_table, dsp_index);
    if (dsp == NULL)
        return false;

    // Update Device
    if (!Device_set_key((Device*)dsp, params->subkey, params->sr))
    {
        set_error(params);
        return false;
    }

    // Notify Device state
    Device_set_state_key(
            (Device*)dsp,
            Player_get_device_states(params->handle->player),
            params->subkey);

    return true;
}


static bool read_effect_dsp_impl_key(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);
    assert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "i/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_effect_dsp_impl_conf_key(&hack_params, eff_table, is_instrument);
}


static bool read_effect_dsp_conf_key(
        Reader_params* params, Effect_table* eff_table, bool is_instrument)
{
    assert(params != NULL);
    assert(eff_table != NULL);
    assert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "c/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_effect_dsp_impl_conf_key(&hack_params, eff_table, is_instrument);
}


#define acquire_ins_effects(eff_table, params)         \
    if (true)                                          \
    {                                                  \
        int32_t ins_index = -1;                        \
        acquire_ins_index(ins_index, (params));        \
        Instrument* ins = NULL;                        \
        acquire_ins(ins, (params)->handle, ins_index); \
        (eff_table) = Instrument_get_effects(ins);     \
    }                                                  \
    else (void)0


static bool read_ins_effect_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_effect_manifest(params, eff_table, is_instrument);
}


static bool read_ins_effect_in_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_effect_in_port_manifest(params, eff_table, is_instrument);
}


static bool read_ins_effect_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_effect_out_port_manifest(params, eff_table, is_instrument);
}


static bool read_ins_effect_connections(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_effect_connections(params, eff_table, is_instrument);
}


static bool read_ins_dsp_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_dsp_manifest(params, eff_table, is_instrument);
}


static bool read_ins_dsp_in_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_dsp_in_port_manifest(params, eff_table, is_instrument);
}


static bool read_ins_dsp_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_dsp_out_port_manifest(params, eff_table, is_instrument);
}


static bool read_ins_dsp_type(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_dsp_type(params, eff_table, is_instrument);
}


static bool read_ins_dsp_impl_key(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_dsp_impl_key(params, eff_table, is_instrument);
}


static bool read_ins_dsp_conf_key(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = NULL;
    acquire_ins_effects(eff_table, params);

    const bool is_instrument = true;

    return read_effect_dsp_conf_key(params, eff_table, is_instrument);
}


static bool read_effect_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_effect_manifest(params, eff_table, is_instrument);
}


static bool read_effect_in_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_effect_in_port_manifest(params, eff_table, is_instrument);
}


static bool read_effect_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_effect_out_port_manifest(params, eff_table, is_instrument);
}


static bool read_effect_connections(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_effect_connections(params, eff_table, is_instrument);
}


static bool read_dsp_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_dsp_manifest(params, eff_table, is_instrument);
}


static bool read_dsp_in_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_dsp_in_port_manifest(params, eff_table, is_instrument);
}


static bool read_dsp_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_dsp_out_port_manifest(params, eff_table, is_instrument);
}


static bool read_dsp_type(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_dsp_type(params, eff_table, is_instrument);
}


static bool read_dsp_impl_key(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_dsp_impl_key(params, eff_table, is_instrument);
}


static bool read_dsp_conf_key(Reader_params* params)
{
    assert(params != NULL);

    Effect_table* eff_table = Module_get_effects(Handle_get_module(params->handle));
    const bool is_instrument = false;

    return read_effect_dsp_conf_key(params, eff_table, is_instrument);
}


#define acquire_pattern(pattern, handle, index)                         \
    if (true)                                                           \
    {                                                                   \
        Module* module = Handle_get_module((handle));                   \
        (pattern) = Pat_table_get(Module_get_pats(module), (index));    \
        if ((pattern) == NULL)                                          \
        {                                                               \
            (pattern) = new_Pattern();                                  \
            if ((pattern) == NULL || !Pat_table_set(                    \
                        Module_get_pats(module), (index), (pattern)))   \
            {                                                           \
                del_Pattern((pattern));                                 \
                Handle_set_error((handle), ERROR_MEMORY,                \
                        "Could not allocate memory for a new pattern"); \
                return false;                                           \
            }                                                           \
        }                                                               \
    } else (void)0


#define acquire_pattern_index(index, params)            \
    if (true)                                           \
    {                                                   \
        (index) = (params)->indices[0];                 \
        if ((index) < 0 || (index) >= KQT_PATTERNS_MAX) \
            return true;                                \
    }                                                   \
    else (void)0


static bool read_pattern_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_pattern_index(index, params);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Pat_table* pats = Module_get_pats(Handle_get_module(params->handle));
    Pat_table_set_existent(pats, index, existent);

    return true;
}


static bool read_pattern(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_pattern_index(index, params);

    Pattern* pattern = NULL;
    acquire_pattern(pattern, params->handle, index);

    if (!Pattern_parse_header(pattern, params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_column(Reader_params* params)
{
    assert(params != NULL);

    int32_t pat_index = -1;
    acquire_pattern_index(pat_index, params);

    const int32_t col_index = params->indices[1];
    if (col_index < 0 || col_index >= KQT_COLUMNS_MAX)
        return true;

    Pattern* pattern = NULL;
    acquire_pattern(pattern, params->handle, pat_index);

    const Event_names* event_names =
            Event_handler_get_names(Player_get_event_handler(params->handle->player));
    Column* column = new_Column_from_string(
            params->sr, Pattern_get_length(pattern), event_names);
    if (column == NULL)
    {
        set_error(params);
        return false;
    }

    if (!Pattern_set_column(pattern, col_index, column))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Could not allocate memory for a new column");
        return false;
    }

    return true;
}


static bool read_pat_instance_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t pat_index = -1;
    acquire_pattern_index(pat_index, params);

    const int32_t pinst_index = params->indices[1];
    if (pinst_index < 0 || pinst_index >= KQT_PAT_INSTANCES_MAX)
        return true;

    Pattern* pattern = NULL;
    acquire_pattern(pattern, params->handle, pat_index);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Pattern_set_inst_existent(pattern, pinst_index, existent);

    return true;
}


static bool read_scale(Reader_params* params)
{
    assert(params != NULL);

    const int32_t index = params->indices[0];

    if (index < 0 || index >= KQT_SCALES_MAX)
        return true;

    Scale* scale = new_Scale_from_string(params->sr);
    if (scale == NULL)
    {
        set_error(params);
        return false;
    }

    Module_set_scale(Handle_get_module(params->handle), params->indices[0], scale);

    return true;
}


#define acquire_song_index(index, params)            \
    if (true)                                        \
    {                                                \
        (index) = (params)->indices[0];              \
        if ((index) < 0 || (index) >= KQT_SONGS_MAX) \
            return true;                             \
    }                                                \
    else (void)0


static bool read_song_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_song_index(index, params);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Module* module = Handle_get_module(params->handle);
    Song_table_set_existent(module->songs, index, existent);

    return true;
}


static bool read_song(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_song_index(index, params);

    Song* song = new_Song_from_string(params->sr);
    if (song == NULL)
    {
        set_error(params);
        return false;
    }

    Song_table* st = Module_get_songs(Handle_get_module(params->handle));
    if (!Song_table_set(st, index, song))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory");
        del_Song(song);
        return false;
    }

    return true;
}


static bool read_song_order_list(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_song_index(index, params);

    Order_list* ol = new_Order_list(params->sr);
    if (ol == NULL)
    {
        set_error(params);
        return false;
    }

    Module* module = Handle_get_module(params->handle);

    if (module->order_lists[index] != NULL)
        del_Order_list(module->order_lists[index]);

    module->order_lists[index] = ol;

    return true;
}


