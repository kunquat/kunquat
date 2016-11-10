

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/Parse_manager.h>

#include <debug/assert.h>
#include <Handle_private.h>
#include <init/Bind.h>
#include <init/Connections.h>
#include <init/devices/Au_control_vars.h>
#include <init/devices/Au_expressions.h>
#include <init/devices/Au_streams.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Device_params.h>
#include <init/devices/Param_proc_filter.h>
#include <init/devices/Proc_type.h>
#include <init/Environment.h>
#include <init/manifest.h>
#include <init/sheet/Channel_defaults_list.h>
#include <memory.h>
#include <string/common.h>
#include <string/key_pattern.h>
#include <string/Streader.h>

#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


typedef struct Reader_params
{
    Handle* handle;
    const int32_t* indices;
    const char* subkey;
    Streader* sr;
} Reader_params;


#define MODULE_KEYP(name, keyp, def) static bool read_##name(Reader_params* params);
#include <init/module_key_patterns.h>


static const struct
{
    const char* keyp;
    bool (*func)(Reader_params*);
} keyp_to_func[] =
{
#define MODULE_KEYP(name, keyp, def) { keyp, read_##name, },
#include <init/module_key_patterns.h>
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
    } else ignore(0)


static const Audio_unit* find_au(Handle* handle, int32_t au_index, int32_t sub_au_index)
{
    rassert(handle != NULL);

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
    rassert(handle != NULL);

    const Audio_unit* au = find_au(handle, au_index, sub_au_index);
    if (au == NULL)
        return false;

    return Device_get_port_existence((const Device*)au, DEVICE_PORT_TYPE_RECV, port);
}


static bool is_au_out_conn_possible(
        Handle* handle, int32_t au_index, int32_t sub_au_index, int32_t port)
{
    rassert(handle != NULL);

    const Audio_unit* au = find_au(handle, au_index, sub_au_index);
    if (au == NULL)
        return false;

    return Device_get_port_existence((const Device*)au, DEVICE_PORT_TYPE_SEND, port);
}


static const Processor* find_complete_proc(
        Handle* handle, int32_t au_index, int32_t sub_au_index, int32_t proc_index)
{
    rassert(handle != NULL);

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
    rassert(handle != NULL);

    const Processor* proc = find_complete_proc(
            handle, au_index, sub_au_index, proc_index);
    if (proc == NULL)
        return false;

    return Device_get_port_existence((const Device*)proc, DEVICE_PORT_TYPE_RECV, port);
}


static bool is_proc_out_conn_possible(
        Handle* handle,
        int32_t au_index,
        int32_t sub_au_index,
        int32_t proc_index,
        int32_t port)
{
    rassert(handle != NULL);

    const Processor* proc = find_complete_proc(
            handle, au_index, sub_au_index, proc_index);
    if (proc == NULL)
        return false;

    return Device_get_port_existence((const Device*)proc, DEVICE_PORT_TYPE_SEND, port);
}


static bool is_connection_possible(
        Handle* handle, const char* keyp, const Key_indices indices)
{
    rassert(handle != NULL);
    rassert(keyp != NULL);
    rassert(indices != NULL);

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


bool parse_data(Handle* handle, const char* key, const void* data, long length)
{
//    fprintf(stderr, "parsing %s\n", key);
    rassert(handle != NULL);
    check_key(handle, key, false);
    rassert(data != NULL || length == 0);
    rassert(length >= 0);

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
        rassert(false);
        return false;
    }

    rassert(strlen(key) == strlen(key_pattern));

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


static bool read_dc_blocker_enabled(Reader_params* params)
{
    rassert(params != NULL);

    Module* module = Handle_get_module(params->handle);
    const bool was_enabled = module->is_dc_blocker_enabled;

    if (!Module_read_dc_blocker_enabled(module, params->sr))
    {
        set_error(params);
        return false;
    }

    // Prevent audible clicks
    if (!was_enabled && module->is_dc_blocker_enabled)
        Player_reset_dc_blocker(params->handle->player);

    return true;
}


static bool read_mixing_volume(Reader_params* params)
{
    rassert(params != NULL);

    if (!Module_read_mixing_volume(Handle_get_module(params->handle), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_force_shift(Reader_params* params)
{
    rassert(params != NULL);

    if (!Module_read_force_shift(Handle_get_module(params->handle), params->sr))
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
    else ignore(0)


static bool read_out_port_manifest(Reader_params* params)
{
    rassert(params != NULL);

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
            (Device*)module, DEVICE_PORT_TYPE_RECV, out_port_index, existent);

    return true;
}


static bool read_connections(Reader_params* params)
{
    rassert(params != NULL);

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
    rassert(params != NULL);

    if (!Module_set_au_map(Handle_get_module(params->handle), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_control_manifest(Reader_params* params)
{
    rassert(params != NULL);

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
    rassert(params != NULL);

    if (!Module_parse_random_seed(Handle_get_module(params->handle), params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_environment(Reader_params* params)
{
    rassert(params != NULL);

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
    rassert(params != NULL);

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


static bool read_ch_defaults(Reader_params* params)
{
    rassert(params != NULL);

    Channel_defaults_list* ch_defs = new_Channel_defaults_list(params->sr);
    if (ch_defs == NULL)
    {
        set_error(params);
        return false;
    }

    Module* module = Handle_get_module(params->handle);
    Module_set_ch_defaults_list(module, ch_defs);

    return true;
}


static bool read_album_manifest(Reader_params* params)
{
    rassert(params != NULL);

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
    rassert(params != NULL);

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
    rassert(handle != NULL);
    rassert(au_table != NULL);
    rassert(index >= 0);
    rassert(index < KQT_AUDIO_UNITS_MAX);

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
        rassert(au_devices[i] != NULL);
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

    // Add Device states to the audio unit state for rendering purposes
    // TODO: fix this mess, Au states should probably contain their own Device states
    {
        Device_states* dstates = Player_get_device_states(handle->player);
        uint32_t au_id = Device_get_id((const Device*)au);
        Au_state* au_state = (Au_state*)Device_states_get_state(dstates, au_id);
        Au_state_set_device_states(au_state, dstates);
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
    else ignore(0)


#define acquire_au(au, handle, au_table, index)               \
    if (true)                                                 \
    {                                                         \
        (au) = add_audio_unit((handle), (au_table), (index)); \
        if ((au) == NULL)                                     \
            return false;                                     \
    }                                                         \
    else ignore(0)


typedef struct amdata
{
    Handle* handle;
    Au_type type;
} amdata;


static bool read_au_manifest_entry(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

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
    rassert(params != NULL);

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


static bool read_any_au_in_port_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

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
            (Device*)au, DEVICE_PORT_TYPE_RECV, in_port_index, existent);

    return true;
}


static bool read_any_au_out_port_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

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
            (Device*)au, DEVICE_PORT_TYPE_SEND, out_port_index, existent);

    return true;
}


static bool read_any_au_connections(Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

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


static bool read_any_au_control_vars(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (Streader_has_data(params->sr))
    {
        Au_control_vars* au_control_vars = new_Au_control_vars(params->sr);
        if (au_control_vars == NULL)
        {
            set_error(params);
            return false;
        }

        Audio_unit_set_control_vars(au, au_control_vars);

        if (level == 0)
        {
            if (!Player_alloc_channel_cv_state(params->handle->player, au_control_vars))
            {
                Handle_set_error(params->handle, ERROR_MEMORY,
                        "Could not allocate memory for audio unit control variables");
                return false;
            }
        }
    }
    else
    {
        Audio_unit_set_control_vars(au, NULL);
    }

    return true;
}


static bool read_any_au_streams(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (Streader_has_data(params->sr))
    {
        Au_streams* au_streams = new_Au_streams(params->sr);
        if (au_streams == NULL)
        {
            set_error(params);
            return false;
        }

        Audio_unit_set_streams(au, au_streams);

        if (level == 0)
        {
            if (!Player_alloc_channel_streams(params->handle->player, au_streams))
            {
                Handle_set_error(params->handle, ERROR_MEMORY,
                        "Could not allocate memory for audio unit streams");
                return false;
            }
        }
    }
    else
    {
        Audio_unit_set_streams(au, NULL);
    }

    return true;
}


static bool read_any_au_hit_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

    if (level > 0)
        return true;

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    const int32_t hit_index = params->indices[1];
    if ((hit_index < 0) || (hit_index >= KQT_HITS_MAX))
        return true;

    const bool existence = read_default_manifest(params->sr);
    if (Streader_is_error_set(params->sr))
    {
        set_error(params);
        return false;
    }

    Audio_unit_set_hit_existence(au, hit_index, existence);

    return true;
}


static bool read_any_au_hit_proc_filter(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

    if (level > 0)
        return true;

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    const int32_t hit_index = params->indices[1];
    if ((hit_index < 0) || (hit_index >= KQT_HITS_MAX))
        return true;

    if (Streader_has_data(params->sr))
    {
        Param_proc_filter* hpf = new_Param_proc_filter(params->sr);
        if (hpf == NULL)
        {
            set_error(params);
            return false;
        }

        Audio_unit_set_hit_proc_filter(au, hit_index, hpf);
    }
    else
    {
        Audio_unit_set_hit_proc_filter(au, hit_index, NULL);
    }

    return true;
}


static bool read_any_au_expressions(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

    if (level > 0)
        return true;

    int32_t index = -1;
    acquire_au_index(index, params, level);

    Audio_unit* au = NULL;
    acquire_au(au, params->handle, au_table, index);

    if (Streader_has_data(params->sr))
    {
        Au_expressions* ae = new_Au_expressions(params->sr);
        if (ae == NULL)
        {
            set_error(params);
            return false;
        }

        Audio_unit_set_expressions(au, ae);
    }
    else
    {
        Audio_unit_set_expressions(au, NULL);
    }

    return true;
}


static Processor* add_processor(
        Handle* handle, Audio_unit* au, Proc_table* proc_table, int proc_index)
{
    rassert(handle != NULL);
    rassert(au != NULL);
    rassert(proc_table != NULL);
    rassert(proc_index >= 0);
    rassert(proc_index < KQT_PROCESSORS_MAX);

    static const char* memory_error_str =
        "Couldn't allocate memory for a new processor";

    // Return existing processor
    Processor* proc = Proc_table_get_proc_mut(proc_table, proc_index);
    if (proc != NULL)
        return proc;

    // Create new processor
    proc = new_Processor(proc_index, Audio_unit_get_params(au));
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
    else ignore(0)


static bool read_any_proc_in_port_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

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
            (Device*)proc, DEVICE_PORT_TYPE_RECV, in_port_index, existent);

    return true;
}


static bool read_any_proc_out_port_manifest(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

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
} pmdata;


static bool read_proc_manifest_entry(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

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
    }

    return true;
}


static bool read_any_proc_manifest(Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

    int32_t au_index = -1;
    acquire_au_index(au_index, params, level);
    int32_t proc_index = -1;
    acquire_proc_index(proc_index, params, level + 1);

    if (!Streader_has_data(params->sr))
    {
        // Remove processor
        Audio_unit* au = Au_table_get(au_table, au_index);
        if (au == NULL)
            return true;

        Proc_table* proc_table = Audio_unit_get_procs(au);

        const Processor* proc = Proc_table_get_proc(proc_table, proc_index);
        if (proc != NULL)
        {
            // Remove Device state of the processor
            Device_states* dstates = Player_get_device_states(params->handle->player);
            Device_states_remove_state(dstates, Device_get_id((const Device*)proc));
        }

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
    pmdata* d = &(pmdata){ .handle = params->handle, .cons = NULL };
    if (!Streader_read_dict(params->sr, read_proc_manifest_entry, d))
        return false;

    if (d->cons == NULL)
        return false;

    rassert(d->cons != NULL);
    Device_impl* proc_impl = d->cons();
    if (proc_impl == NULL)
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Couldn't allocate memory for processor implementation");
        return false;
    }

    Device_set_impl((Device*)proc, proc_impl);

    // Remove old Processor Device state
    Device_states* dstates = Player_get_device_states(params->handle->player);
    Device_states_remove_state(dstates, Device_get_id((Device*)proc));

    // Allocate Voice state space
    {
        const int32_t size = Device_impl_get_vstate_size(proc_impl);
        if (!Player_reserve_voice_state_space(params->handle->player, size) ||
                !Player_reserve_voice_state_space(params->handle->length_counter, size))
        {
            Handle_set_error(params->handle, ERROR_MEMORY,
                    "Could not allocate memory for processor voice states");
            return false;
        }
    }

    // Allocate Voice work buffers
    {
        const int32_t audio_rate = Player_get_audio_rate(params->handle->player);
        const int32_t cur_size =
            Player_get_voice_work_buffer_size(params->handle->player);
        const int32_t req_size = Device_impl_get_voice_wb_size(proc_impl, audio_rate);
        if (req_size > cur_size)
        {
            if (!Player_reserve_voice_work_buffer_space(
                        params->handle->player, req_size))
            {
                Handle_set_error(params->handle, ERROR_MEMORY,
                        "Could not allocate memory for voice work buffers");
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
                (Device*)proc, Player_get_device_states(params->handle->player)))
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
    rassert(params != NULL);
    rassert(au_table != NULL);

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

    params->handle->update_connections = true;

    return true;
}


static bool read_any_proc_impl_conf_key(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);

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
    rassert(params != NULL);
    rassert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "i/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_any_proc_impl_conf_key(&hack_params, au_table, level);
}


static bool read_any_proc_conf_key(
        Reader_params* params, Au_table* au_table, int level)
{
    rassert(params != NULL);
    rassert(strlen(params->subkey) < KQT_KEY_LENGTH_MAX - 2);

    char hack_subkey[KQT_KEY_LENGTH_MAX] = "c/";
    strcat(hack_subkey, params->subkey);
    Reader_params hack_params = *params;
    hack_params.subkey = hack_subkey;

    return read_any_proc_impl_conf_key(&hack_params, au_table, level);
}


#define MAKE_AU_EFFECT_READER(base_name)                                \
    static bool read_au_ ## base_name(Reader_params* params)            \
    {                                                                   \
        rassert(params != NULL);                                        \
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
        rassert(params != NULL);                            \
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
#include <init/module_key_patterns.h>


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
    } else ignore(0)


#define acquire_pattern_index(index, params)            \
    if (true)                                           \
    {                                                   \
        (index) = (params)->indices[0];                 \
        if ((index) < 0 || (index) >= KQT_PATTERNS_MAX) \
            return true;                                \
    }                                                   \
    else ignore(0)


static bool read_pattern_manifest(Reader_params* params)
{
    rassert(params != NULL);

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


static bool read_pattern_length(Reader_params* params)
{
    rassert(params != NULL);

    int32_t index = -1;
    acquire_pattern_index(index, params);

    Pattern* pattern = NULL;
    acquire_pattern(pattern, params->handle, index);

    if (!Pattern_read_length(pattern, params->sr))
    {
        set_error(params);
        return false;
    }

    return true;
}


static bool read_column(Reader_params* params)
{
    rassert(params != NULL);

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
    rassert(params != NULL);

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


static bool read_tuning_table(Reader_params* params)
{
    rassert(params != NULL);

    const int32_t index = params->indices[0];

    if (index < 0 || index >= KQT_TUNING_TABLES_MAX)
        return true;

    Tuning_table* tt = NULL;
    if (Streader_has_data(params->sr))
    {
        tt = new_Tuning_table_from_string(params->sr);
        if (tt == NULL)
        {
            set_error(params);
            return false;
        }
    }

    Module_set_tuning_table(Handle_get_module(params->handle), params->indices[0], tt);

    // Create new Tuning state
    if (!Player_create_tuning_state(params->handle->player, params->indices[0]))
    {
        Handle_set_error(params->handle, ERROR_MEMORY,
                "Could not allocate memory for tuning state");
        return false;
    }

    return true;
}


#define acquire_song_index(index, params)            \
    if (true)                                        \
    {                                                \
        (index) = (params)->indices[0];              \
        if ((index) < 0 || (index) >= KQT_SONGS_MAX) \
            return true;                             \
    }                                                \
    else ignore(0)


static bool read_song_manifest(Reader_params* params)
{
    rassert(params != NULL);

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


static Song* add_song(Handle* handle, int index)
{
    rassert(handle != NULL);
    rassert(index >= 0);
    rassert(index < KQT_SONGS_MAX);

    Song_table* st = Module_get_songs(Handle_get_module(handle));
    Song* song = Song_table_get(st, index);
    if (song != NULL)
        return song;

    Song* new_song = new_Song();
    if ((new_song == NULL) || !Song_table_set(st, index, new_song))
    {
        Handle_set_error(
                handle, ERROR_MEMORY, "Could not allocate memory for a new song");
        del_Song(new_song);
        return NULL;
    }

    return new_song;
}


#define acquire_song(song, handle, index)       \
    if (true)                                   \
    {                                           \
        (song) = add_song((handle), (index));   \
        if ((song) == NULL)                     \
            return false;                       \
    }                                           \
    else ignore(0)


static bool read_song_tempo(Reader_params* params)
{
    rassert(params != NULL);

    int32_t index = -1;
    acquire_song_index(index, params);

    Song* song = NULL;
    acquire_song(song, params->handle, index);

    return Song_read_tempo(song, params->sr);
}


static bool read_song_order_list(Reader_params* params)
{
    rassert(params != NULL);

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


