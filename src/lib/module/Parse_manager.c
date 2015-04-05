

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <devices/Audio_unit.h>
#include <devices/Device_params.h>
#include <devices/Proc_type.h>
#include <Handle_private.h>
#include <memory.h>
#include <module/Bind.h>
#include <module/Environment.h>
#include <module/manifest.h>
#include <module/Parse_manager.h>
#include <module/sheet/Channel_defaults_list.h>
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


static const Audio_unit* find_au(Handle* handle, int32_t au_index, int32_t sub_au_index)
{
    assert(handle != NULL);

    const Module* module = Handle_get_module(handle);

    Audio_unit* au = Au_table_get(Module_get_au_table(module), au_index);
    if (au == NULL)
        return NULL;

    if (sub_au_index >= 0)
    {
        Au_table* au_table = Audio_unit_get_au_table(au);
        if (au_table == NULL)
            return NULL;

        au = Au_table_get(au_table, sub_au_index);
    }

    return au;
}


static bool is_au_in_conn_possible(
        Handle* handle, int32_t au_index, int32_t sub_au_index, int32_t port)
{
    assert(handle != NULL);

    const Audio_unit* au = find_au(handle, au_index, sub_au_index);
    if (au == NULL)
        return false;

    return Device_get_port_existence((const Device*)au, DEVICE_PORT_TYPE_RECEIVE, port);
}


static bool is_au_out_conn_possible(
        Handle* handle, int32_t au_index, int32_t sub_au_index, int32_t port)
{
    assert(handle != NULL);

    const Audio_unit* au = find_au(handle, au_index, sub_au_index);
    if (au == NULL)
        return false;

    return Device_get_port_existence((const Device*)au, DEVICE_PORT_TYPE_SEND, port);
}


static const Processor* find_complete_proc(
        Handle* handle, int32_t au_index, int32_t sub_au_index, int32_t proc_index)
{
    assert(handle != NULL);

    const Audio_unit* au = find_au(handle, au_index, sub_au_index);
    if (au == NULL)
        return false;

    const Processor* proc = Audio_unit_get_proc(au, proc_index);
    if ((proc == NULL) || !Device_has_complete_type((const Device*)proc))
        return NULL;

    return proc;
}


static bool is_proc_in_conn_possible(
        Handle* handle,
        int32_t au_index,
        int32_t sub_au_index,
        int32_t proc_index,
        int32_t port)
{
    assert(handle != NULL);

    const Processor* proc = find_complete_proc(
            handle, au_index, sub_au_index, proc_index);
    if (proc == NULL)
        return false;

    return Device_get_port_existence(
            (const Device*)proc, DEVICE_PORT_TYPE_RECEIVE, port);
}


static bool is_proc_out_conn_possible(
        Handle* handle,
        int32_t au_index,
        int32_t sub_au_index,
        int32_t proc_index,
        int32_t port)
{
    assert(handle != NULL);

    const Processor* proc = find_complete_proc(
            handle, au_index, sub_au_index, proc_index);
    if (proc == NULL)
        return false;

    return Device_get_port_existence((const Device*)proc, DEVICE_PORT_TYPE_SEND, port);
}


static bool is_connection_possible(
        Handle* handle, const char* keyp, const Key_indices indices)
{
    assert(handle != NULL);
    assert(keyp != NULL);
    assert(indices != NULL);

    if (string_has_prefix(keyp, "au_XX/au_XX/proc_XX/in_XX/"))
        return is_proc_in_conn_possible(
                handle, indices[0], indices[1], indices[2], indices[3]);
    else if (string_has_prefix(keyp, "au_XX/au_XX/proc_XX/out_XX/"))
        return is_proc_out_conn_possible(
                handle, indices[0], indices[1], indices[2], indices[3]);
    else if (string_has_prefix(keyp, "au_XX/au_XX/in_XX/"))
        return is_au_in_conn_possible(handle, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "au_XX/au_XX/out_XX/"))
        return is_au_out_conn_possible(handle, indices[0], indices[1], indices[2]);
    else if (string_has_prefix(keyp, "au_XX/proc_XX/in_XX/"))
        return is_proc_in_conn_possible(handle, indices[0], -1, indices[1], indices[2]);
    else if (string_has_prefix(keyp, "au_XX/proc_XX/out_XX/"))
        return is_proc_out_conn_possible(handle, indices[0], -1, indices[1], indices[2]);
    else if (string_has_prefix(keyp, "au_XX/in_XX/"))
        return is_au_in_conn_possible(handle, indices[0], -1, indices[1]);
    else if (string_has_prefix(keyp, "au_XX/out_XX/"))
        return is_au_out_conn_possible(handle, indices[0], -1, indices[1]);

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


#define acquire_port_index(index, params, depth)            \
    if (true)                                               \
    {                                                       \
        (index) = (params)->indices[(depth)];               \
        if ((index) < 0 || (index) >= KQT_DEVICE_PORTS_MAX) \
            return true;                                    \
    }                                                       \
    else (void)0


static bool read_out_port_manifest(Reader_params* params)
{
    assert(params != NULL);

    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, 0);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Module* module = Handle_get_module(params->handle);
    Device_set_port_existence(
            (Device*)module, DEVICE_PORT_TYPE_RECEIVE, out_port_index, existent);

    return true;
}


static bool read_connections(Reader_params* params)
{
    assert(params != NULL);

    Module* module = Handle_get_module(params->handle);

    Connections* graph = new_Connections_from_string(
            params->sr,
            CONNECTION_LEVEL_GLOBAL,
            Module_get_au_table(module),
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

    if (!Module_set_au_map(Handle_get_module(params->handle), params->sr))
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


static Audio_unit* add_audio_unit(Handle* handle, Au_table* au_table, int index)
{
    assert(handle != NULL);
    assert(au_table != NULL);
    assert(index >= 0);
    assert(index < KQT_AUDIO_UNITS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new audio unit";

    // Return existing audio unit
    Audio_unit* au = Au_table_get(au_table, index);
    if (au != NULL)
        return au;

    // Create new audio unit
    au = new_Audio_unit();
    if (au == NULL || !Au_table_set(au_table, index, au))
    {
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_Audio_unit(au);
        return NULL;
    }

    // Allocate Device states for the new audio unit
    const Device* au_devices[] =
    {
        (const Device*)au,
        Audio_unit_get_input_interface(au),
        Audio_unit_get_output_interface(au),
        NULL
    };
    for (int i = 0; i < 3; ++i)
    {
        assert(au_devices[i] != NULL);
        Device_state* ds = Device_create_state(
                au_devices[i],
                Player_get_audio_rate(handle->player),
                Player_get_audio_buffer_size(handle->player));
        if (ds == NULL || !Device_states_add_state(
                    Player_get_device_states(handle->player), ds))
        {
            del_Device_state(ds);
            Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
            Au_table_remove(au_table, index);
            return NULL;
        }
    }

    return au;
}


#define acquire_au_index(index, params, level)             \
    if (true)                                              \
    {                                                      \
        (index) = (params)->indices[(level)];              \
        if ((index) < 0 || (index) >= KQT_AUDIO_UNITS_MAX) \
            return true;                                   \
    }                                                      \
    else (void)0


#define acquire_au(au, handle, au_table, index)               \
    if (true)                                                 \
    {                                                         \
        (au) = add_audio_unit((handle), (au_table), (index)); \
        if ((au) == NULL)                                     \
            return false;                                     \
    }                                                         \
    else (void)0


typedef struct amdata
{
    Handle* handle;
    Au_type type;
} amdata;


static bool read_au_manifest_entry(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    amdata* d = userdata;

    if (string_eq(key, "type"))
    {
        char type[64] = "";
        if (!Streader_read_string(sr, 64, type))
            return false;

        if (string_eq(type, "instrument"))
        {
            d->type = AU_TYPE_INSTRUMENT;
        }
        else if (string_eq(type, "effect"))
        {
            d->type = AU_TYPE_EFFECT;
        }
        else
        {
            Handle_set_error(d->handle, ERROR_FORMAT,
                    "Unsupported Audio unit type: %s", type);
            return false;
        }
    }

    return true;
}


static bool read_any_au_manifest(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (!Streader_has_data(params->sr))
    {
        Device_set_existent((Device*)au, false);
        return true;
    }

    amdata* d = &(amdata){ .handle = params->handle, .type = AU_TYPE_INVALID };
    if (!Streader_read_dict(params->sr, read_au_manifest_entry, d))
        return false;

    if (d->type == AU_TYPE_INVALID)
        return false;

    Audio_unit_set_type(au, d->type);
    Device_set_existent((Device*)au, true);

    return true;
}


static bool read_any_au(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (!Audio_unit_parse_header(au, params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_any_au_in_port_manifest(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t in_port_index = -1;
    acquire_port_index(in_port_index, params, level + 1);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);

    Device_set_port_existence(
            (Device*)au, DEVICE_PORT_TYPE_RECEIVE, in_port_index, existent);
    Device_set_port_existence(
            (Device*)au, DEVICE_PORT_TYPE_SEND, in_port_index, existent);

    return true;
}


static bool read_any_au_out_port_manifest(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, level + 1);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);

    Device_set_port_existence(
            (Device*)au, DEVICE_PORT_TYPE_RECEIVE, out_port_index, existent);
    Device_set_port_existence(
            (Device*)au, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


static bool read_any_au_connections(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (!Streader_has_data(params->sr))
    {
        Audio_unit_set_connections(au, NULL);
        params->handle->update_connections = true;
    }
    else
    {
        Connections* graph = new_Connections_from_string(
                params->sr, CONNECTION_LEVEL_AU, Audio_unit_get_au_table(au), (Device*)au);
        if (graph == NULL)
        {
            set_error(params);
            return false;
        }

        Audio_unit_set_connections(au, graph);
        params->handle->update_connections = true;
    }

    return true;
}


static bool read_any_au_env_generic(
        Reader_params* params,
        Au_table* au_table,
        int level,
        bool parse_func(Au_params*, Streader*))
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (!parse_func(Audio_unit_get_params(au), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_any_au_env_force(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    return read_any_au_env_generic(params, au_table, level, Au_params_parse_env_force);
}


static bool read_any_au_env_force_release(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    return read_any_au_env_generic(
            params, au_table, level, Au_params_parse_env_force_rel);
}


static bool read_any_au_env_force_filter(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    return read_any_au_env_generic(
            params, au_table, level, Au_params_parse_env_force_filter);
}


static bool read_any_au_env_pitch_pan(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    return read_any_au_env_generic(
            params, au_table, level, Au_params_parse_env_pitch_pan);
}


static Processor* add_processor(
        Handle* handle,
        Audio_unit* au,
        Proc_table* proc_table,
        int proc_index)
{
    assert(handle != NULL);
    assert(au != NULL);
    assert(proc_table != NULL);
    assert(proc_index >= 0);
    assert(proc_index < KQT_PROCESSORS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new processor";

    // Return existing processor
    Processor* proc = Proc_table_get_proc_mut(proc_table, proc_index);
    if (proc != NULL)
        return proc;

    // Create new processor
    proc = new_Processor(Audio_unit_get_params(au));
    if (proc == NULL || !Proc_table_set_proc(proc_table, proc_index, proc))
    {
        Handle_set_error(handle, ERROR_MEMORY, memory_error_str);
        del_Processor(proc);
        return NULL;
    }

    return proc;
}


#define acquire_proc_index(index, params, level)          \
    if (true)                                             \
    {                                                     \
        (index) = (params)->indices[(level)];             \
        if ((index) < 0 || (index) >= KQT_PROCESSORS_MAX) \
            return true;                                  \
    }                                                     \
    else (void)0


static bool read_any_proc_in_port_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);
    int32_t in_port_index = -1;
    acquire_port_index(in_port_index, params, level + 2);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);
    Proc_table* proc_table = Audio_unit_get_procs(au);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Processor* proc = add_processor(params->handle, au, proc_table, proc_index);
    if (proc == NULL)
        return false;

    Device_set_port_existence(
            (Device*)proc, DEVICE_PORT_TYPE_RECEIVE, in_port_index, existent);

    return true;
}


static bool read_any_proc_out_port_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);
    int32_t out_port_index = -1;
    acquire_port_index(out_port_index, params, level + 2);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);
    Proc_table* proc_table = Audio_unit_get_procs(au);

    const bool existent = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Processor* proc = add_processor(params->handle, au, proc_table, proc_index);
    if (proc == NULL)
        return false;

    Device_set_port_existence(
            (Device*)proc, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


typedef struct pmdata
{
    Handle* handle;
    Proc_cons* cons;
    Proc_property* prop;
} pmdata;


static bool read_proc_manifest_entry(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    pmdata* d = userdata;

    if (string_eq(key, "type"))
    {
        char type[PROC_TYPE_LENGTH_MAX] = "";
        if (!Streader_read_string(sr, PROC_TYPE_LENGTH_MAX, type))
            return false;

        d->cons = Proc_type_find_cons(type);
        if (d->cons == NULL)
        {
            Handle_set_error(d->handle, ERROR_FORMAT,
                    "Unsupported Processor type: %s", type);
            return false;
        }

        d->prop = Proc_type_find_property(type);
    }

    return true;
}


static bool read_any_proc_manifest(Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);

    if (!Streader_has_data(params->sr))
    {
        // Remove processor
        Module* module = Handle_get_module(params->handle);
        Audio_unit* au = Au_table_get(Module_get_au_table(module), au_index);
        if (au == NULL)
            return true;

        Proc_table* proc_table = Audio_unit_get_procs(au);
        Proc_table_set_existent(proc_table, proc_index, false);
        Proc_table_remove_proc(proc_table, proc_index);

        return true;
    }

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);
    Proc_table* proc_table = Audio_unit_get_procs(au);

    Processor* proc = add_processor(params->handle, au, proc_table, proc_index);
    if (proc == NULL)
        return false;

    // Create the Processor implementation
    pmdata* d = &(pmdata){ .handle = params->handle, .cons = NULL, .prop = NULL };
    if (!Streader_read_dict(params->sr, read_proc_manifest_entry, d))
        return false;

    if (d->cons == NULL)
        return false;

    assert(d->cons != NULL);
    Device_impl* proc_impl = d->cons(proc);
    if (proc_impl == NULL)
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for processor implementation");
        return false;
    }

    if (!Device_set_impl((Device*)proc, proc_impl))
    {
        del_Device_impl(proc_impl);
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while initialising Processor implementation");
        return false;
    }

    // Remove old Processor Device state
    Device_states* dstates = Player_get_device_states(params->handle->player);
    Device_states_remove_state(dstates, Device_get_id((Device*)proc));

    // Get processor properties
    Proc_property* property = d->prop;
    if (property != NULL)
    {
        // Allocate Voice state space
        const char* size_str = property(proc, "voice_state_size");
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
                        "Couldn't allocate memory for processor voice states");
                del_Device_impl(proc_impl);
                return false;
            }
        }

        // Allocate channel-specific processor state space
        const char* proc_state_vars = property(proc, "proc_state_vars");
        if (proc_state_vars != NULL)
        {
            Streader* gsv_sr = Streader_init(
                    STREADER_AUTO,
                    proc_state_vars,
                    strlen(proc_state_vars));

            if (!Player_alloc_channel_proc_state_keys(
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

    // Allocate Device state(s) for this Processor
    Device_state* ds = Device_create_state(
            (Device*)proc,
            Player_get_audio_rate(params->handle->player),
            Player_get_audio_buffer_size(params->handle->player));
    if (ds == NULL || !Device_states_add_state(dstates, ds))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for device state");
        del_Device_state(ds);
        del_Processor(proc);
        return false;
    }

    // Sync the Processor
    if (!Device_sync((Device*)proc))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing processor");
        return false;
    }

    // Sync the Device state(s)
    if (!Device_sync_states(
                (Device*)proc,
                Player_get_device_states(params->handle->player)))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory while syncing processor");
        return false;
    }

    // Force connection update so that we get buffers for the new Device state(s)
    params->handle->update_connections = true;

    Proc_table_set_existent(proc_table, proc_index, true);

    return true;
}


static bool read_any_proc_signal_type(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    assert(au_table != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);
    Proc_table* proc_table = Audio_unit_get_procs(au);

    Processor* proc = add_processor(params->handle, au, proc_table, proc_index);
    if (proc == NULL)
        return false;

    bool voice_signals_selected = true;
    if (Streader_has_data(params->sr))
    {
        char type_name[64] = "";
        if (!Streader_read_string(params->sr, 64, type_name))
            return false;

        if (string_eq(type_name, "voice"))
            voice_signals_selected = true;
        else if (string_eq(type_name, "mixed"))
            voice_signals_selected = false;
        else
        {
            Handle_set_error(params->handle, ERROR_FORMAT,
                    "Unrecognised processor signal type: %s", type_name);
            return false;
        }
    }

    const bool mixed_signals_selected = !voice_signals_selected;

    Processor_set_voice_signals(proc, voice_signals_selected);
    Device_set_mixed_signals((Device*)proc, mixed_signals_selected);

    return true;
}


static bool read_any_proc_impl_conf_key(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);

    if (!key_is_device_param(params->subkey))
        return true;

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);
    Proc_table* proc_table = Audio_unit_get_procs(au);

    Processor* proc = add_processor(params->handle, au, proc_table, proc_index);
    if (proc == NULL)
        return false;

    // Update Device
    if (!Device_set_key((Device*)proc, params->subkey, params->sr))
    {
        set_error(params);
        return false;
    }

    // Update Device state
    Device_set_state_key(
            (Device*)proc,
            Player_get_device_states(params->handle->player),
            params->subkey);

    return true;
}


static bool read_any_proc_impl_key(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    assert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "i/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_any_proc_impl_conf_key(&hack_params, au_table, level);
}


static bool read_any_proc_conf_key(
        Reader_params* params, Au_table* au_table, int level)
{
    assert(params != NULL);
    assert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "c/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_any_proc_impl_conf_key(&hack_params, au_table, level);
}


static bool read_any_proc_voice_feature(
        Reader_params* params,
        Au_table* au_table,
        int level,
        Voice_feature feature)
{
    assert(params != NULL);
    assert(au_table != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);
    int32_t port_num = -1;
    acquire_port_index(port_num, params, level + 2);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, au_index);
    Proc_table* proc_table = Audio_unit_get_procs(au);

    Processor* proc = add_processor(params->handle, au, proc_table, proc_index);
    if (proc == NULL)
        return false;

    bool feature_enabled = true;
    if (Streader_has_data(params->sr) &&
            !Streader_read_bool(params->sr, &feature_enabled))
        return false;

    Processor_set_voice_feature(proc, port_num, feature, feature_enabled);

    return true;
}


static bool read_any_proc_vf_pitch(
        Reader_params* params, Au_table* au_table, int level)
{
    return read_any_proc_voice_feature(params, au_table, level, VOICE_FEATURE_PITCH);
}


static bool read_any_proc_vf_force(
        Reader_params* params, Au_table* au_table, int level)
{
    return read_any_proc_voice_feature(params, au_table, level, VOICE_FEATURE_FORCE);
}


static bool read_any_proc_vf_cut(
        Reader_params* params, Au_table* au_table, int level)
{
    return read_any_proc_voice_feature(
            params, au_table, level, VOICE_FEATURE_CUT);
}


static bool read_any_proc_vf_filter(
        Reader_params* params, Au_table* au_table, int level)
{
    return read_any_proc_voice_feature(params, au_table, level, VOICE_FEATURE_FILTER);
}


static bool read_any_proc_vf_panning(
        Reader_params* params, Au_table* au_table, int level)
{
    return read_any_proc_voice_feature(params, au_table, level, VOICE_FEATURE_PANNING);
}


#define MAKE_AU_EFFECT_READER(base_name)                                \
    static bool read_au_ ## base_name(Reader_params* params)            \
    {                                                                   \
        assert(params != NULL);                                         \
                                                                        \
        int32_t top_au_index = -1;                                      \
        acquire_au_index(top_au_index, params, 0);                      \
                                                                        \
        Module* module = Handle_get_module(params->handle);             \
        Au_table* top_au_table = Module_get_au_table(module);           \
                                                                        \
        Audio_unit* top_au = NULL;                                      \
        acquire_au(top_au, params->handle, top_au_table, top_au_index); \
                                                                        \
        Au_table* au_table = Audio_unit_get_au_table(top_au);           \
                                                                        \
        return read_any_ ## base_name(params, au_table, 1);             \
    }

#define MAKE_GLOBAL_AU_READER(base_name)                    \
    static bool read_ ## base_name(Reader_params* params)   \
    {                                                       \
        assert(params != NULL);                             \
                                                            \
        Module* module = Handle_get_module(params->handle); \
        Au_table* au_table = Module_get_au_table(module);   \
                                                            \
        return read_any_ ## base_name(params, au_table, 0); \
    }

#define MAKE_AU_READERS(base_name)   \
    MAKE_AU_EFFECT_READER(base_name) \
    MAKE_GLOBAL_AU_READER(base_name)

#define MODULE_KEYP(name, keyp, def_val)
#define MODULE_AU_KEYP(name, keyp, def_val) MAKE_AU_READERS(name)
#include <module/Module_key_patterns.h>


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


static bool read_song_ch_defaults(Reader_params* params)
{
    assert(params != NULL);

    int32_t index = -1;
    acquire_song_index(index, params);

    Channel_defaults_list* cdl = new_Channel_defaults_list(params->sr);
    if (cdl == NULL)
    {
        set_error(params);
        return false;
    }

    Module* module = Handle_get_module(params->handle);
    Module_set_ch_defaults_list(module, index, cdl);

    return true;
}


