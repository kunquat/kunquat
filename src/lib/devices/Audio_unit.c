

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
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Connections.h>
#include <debug/assert.h>
#include <devices/Au_interface.h>
#include <devices/Audio_unit.h>
#include <devices/Device.h>
#include <devices/Proc_table.h>
#include <devices/Processor.h>
#include <memory.h>
#include <module/Au_table.h>
#include <player/Au_state.h>
#include <string/common.h>


struct Audio_unit
{
    Device parent;

    Au_type type;

    Au_interface* out_iface;
    Au_interface* in_iface;
    Connections* connections;

//    double default_force;       ///< Default force.

    int scale_index;            ///< The index of the Scale used (-1 means the default).

    Au_params params;   ///< All the Audio unit parameters that Processors need.

    Proc_table* procs;
    Au_table* au_table;
};


static Device_state* Audio_unit_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Audio_unit_reset(const Device* device, Device_states* dstates);

static bool Audio_unit_set_audio_rate(
        const Device* device, Device_states* dstates, int32_t mix_rate);

static bool Audio_unit_set_buffer_size(
        const Device* device, Device_states* dstates, int32_t size);

static void Audio_unit_update_tempo(
        const Device* device, Device_states* dstates, double tempo);

//static bool Audio_unit_sync(Device* device, Device_states* dstates);

static void Audio_unit_process_signal(
        const Device* device,
        Device_states* dstates,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo);


Audio_unit* new_Audio_unit(void)
{
    Audio_unit* au = memory_alloc_item(Audio_unit);
    if (au == NULL)
        return NULL;

    //fprintf(stderr, "New Audio unit %p\n", (void*)au);
    au->type = AU_TYPE_INVALID;
    au->out_iface = NULL;
    au->in_iface = NULL;
    au->connections = NULL;
    au->procs = NULL;
    au->au_table = NULL;

    if (!Device_init(&au->parent, false))
    {
        memory_free(au);
        return NULL;
    }
    if (Au_params_init(
                &au->params,
                Device_get_id(&au->parent)) == NULL)
    {
        Device_deinit(&au->parent);
        memory_free(au);
        return NULL;
    }

    Device_set_state_creator(&au->parent, Audio_unit_create_state);
    Device_set_reset(&au->parent, Audio_unit_reset);
    Device_register_set_audio_rate(&au->parent, Audio_unit_set_audio_rate);
    Device_register_update_tempo(&au->parent, Audio_unit_update_tempo);
    Device_register_set_buffer_size(&au->parent, Audio_unit_set_buffer_size);
    Device_set_signal_support(&au->parent, true);
    Device_set_process(&au->parent, Audio_unit_process_signal);

    au->out_iface = new_Au_interface();
    au->in_iface = new_Au_interface();
    au->procs = new_Proc_table(KQT_PROCESSORS_MAX);
    au->au_table = new_Au_table(KQT_AUDIO_UNITS_MAX);
    if ((au->out_iface == NULL) ||
            (au->in_iface == NULL) ||
            (au->procs == NULL) ||
            (au->au_table == NULL))
    {
        del_Audio_unit(au);
        return NULL;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_set_port_existence(
                &au->out_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
        Device_set_port_existence(
                &au->in_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
    }
    Device_set_port_existence(
            &au->out_iface->parent, DEVICE_PORT_TYPE_RECEIVE, 0, true);

//    au->default_force = AU_DEFAULT_FORCE;
    au->params.force_variation = AU_DEFAULT_FORCE_VAR;

    au->scale_index = AU_DEFAULT_SCALE_INDEX;

    return au;
}


void Audio_unit_set_type(Audio_unit* au, Au_type type)
{
    assert(au != NULL);
    assert(type != AU_TYPE_INVALID);

    au->type = type;

    return;
}


Au_type Audio_unit_get_type(const Audio_unit* au)
{
    assert(au != NULL);
    return au->type;
}


typedef struct au_params
{
    double global_force;
    double default_force;
    double force_variation;
    int64_t scale_index;
} au_params;

static bool read_au_field(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    au_params* p = userdata;

    if (string_eq(key, "force"))
        Streader_read_float(sr, &p->default_force);
    else if (string_eq(key, "force_variation"))
        Streader_read_float(sr, &p->force_variation);
    else if (string_eq(key, "global_force"))
        Streader_read_float(sr, &p->global_force);
    else if (string_eq(key, "scale"))
    {
        if (!Streader_read_int(sr, &p->scale_index))
            return false;

        if (p->scale_index < -1 || p->scale_index >= KQT_SCALES_MAX)
        {
            Streader_set_error(
                     sr, "Invalid scale index: %" PRId64, p->scale_index);
            return false;
        }
    }
    else
    {
        Streader_set_error(
                sr, "Unsupported field in autrument information: %s", key);
        return false;
    }

    return !Streader_is_error_set(sr);
}

bool Audio_unit_parse_header(Audio_unit* au, Streader* sr)
{
    assert(au != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    au_params* p = &(au_params)
    {
        .global_force = AU_DEFAULT_GLOBAL_FORCE,
        .default_force = AU_DEFAULT_FORCE,
        .force_variation = AU_DEFAULT_FORCE_VAR,
        .scale_index = AU_DEFAULT_SCALE_INDEX,
    };

    if (Streader_has_data(sr) && !Streader_read_dict(sr, read_au_field, p))
        return false;

    au->params.force = p->default_force;
    au->params.force_variation = p->force_variation;
    au->params.global_force = exp2(p->global_force / 6);

    return true;
}


Au_params* Audio_unit_get_params(Audio_unit* au)
{
    assert(au != NULL);
    return &au->params;
}


const Processor* Audio_unit_get_proc(const Audio_unit* au, int index)
{
    assert(au != NULL);
    assert(index >= 0);
    assert(index < KQT_PROCESSORS_MAX);

    return Proc_table_get_proc(au->procs, index);
}


Proc_table* Audio_unit_get_procs(Audio_unit* au)
{
    assert(au != NULL);
    return au->procs;
}


const Audio_unit* Audio_unit_get_au(const Audio_unit* au, int index)
{
    assert(au != NULL);
    assert(au->au_table != NULL);
    assert(index >= 0);
    assert(index < KQT_AUDIO_UNITS_MAX);

    return Au_table_get(au->au_table, index);
}


Au_table* Audio_unit_get_au_table(Audio_unit* au)
{
    assert(au != NULL);
    assert(au->au_table != NULL);

    return au->au_table;
}


void Audio_unit_set_connections(Audio_unit* au, Connections* graph)
{
    assert(au != NULL);

    if (au->connections != NULL)
        del_Connections(au->connections);
    au->connections = graph;

    return;
}


const Connections* Audio_unit_get_connections(const Audio_unit* au)
{
    assert(au != NULL);
    return au->connections;
}


Connections* Audio_unit_get_connections_mut(const Audio_unit* au)
{
    assert(au != NULL);
    return au->connections;
}


bool Audio_unit_prepare_connections(const Audio_unit* au, Device_states* states)
{
    assert(au != NULL);
    assert(states != NULL);

    if (au->connections == NULL)
        return true;

    return Connections_prepare(au->connections, states);
}


const Device* Audio_unit_get_input_interface(const Audio_unit* au)
{
    assert(au != NULL);
    return &au->in_iface->parent;
}


const Device* Audio_unit_get_output_interface(const Audio_unit* au)
{
    assert(au != NULL);
    return &au->out_iface->parent;
}


static Device_state* Audio_unit_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Au_state* is = memory_alloc_item(Au_state);
    if (is == NULL)
        return NULL;

    Device_state_init(&is->parent, device, audio_rate, audio_buffer_size);
    Au_state_reset(is);

    return &is->parent;
}


static void Audio_unit_reset(const Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    const Audio_unit* au = (const Audio_unit*)device;

    // Reset processors
    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(au->procs, i);
        if (proc != NULL)
            Device_reset((const Device*)proc, dstates);
    }

    // Reset internal autruments
    for (int i = 0; i < KQT_AUDIO_UNITS_MAX; ++i)
    {
        const Audio_unit* sub_au = Au_table_get(au->au_table, i);
        if (sub_au != NULL)
            Device_reset((const Device*)sub_au, dstates);
    }

    // Reset autrument state
    Au_state* au_state = (Au_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));
    Au_state_reset(au_state);

    return;
}


static bool Audio_unit_set_audio_rate(
        const Device* device, Device_states* dstates, int32_t audio_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(audio_rate > 0);

    const Audio_unit* au = (const Audio_unit*)device;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(au->procs, i);
        if (proc != NULL &&
                !Device_set_audio_rate((const Device*)proc, dstates, audio_rate))
            return false;
    }

    for (int i = 0; i < KQT_AUDIO_UNITS_MAX; ++i)
    {
        const Audio_unit* sub_au = Au_table_get(au->au_table, i);
        if ((sub_au != NULL) &&
                !Device_set_audio_rate((const Device*)sub_au, dstates, audio_rate))
            return false;
    }

    return true;
}


static void Audio_unit_update_tempo(
        const Device* device, Device_states* dstates, double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    const Audio_unit* au = (const Audio_unit*)device;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(au->procs, i);
        if (proc != NULL)
            Device_update_tempo((const Device*)proc, dstates, tempo);
    }

    for (int i = 0; i < KQT_AUDIO_UNITS_MAX; ++i)
    {
        const Audio_unit* sub_au = Au_table_get(au->au_table, i);
        if (sub_au != NULL)
            Device_update_tempo((const Device*)sub_au, dstates, tempo);
    }

    return;
}


static bool Audio_unit_set_buffer_size(
        const Device* device, Device_states* dstates, int32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    const Audio_unit* au = (const Audio_unit*)device;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(au->procs, i);
        if (proc != NULL &&
                !Device_set_buffer_size((const Device*)proc, dstates, size))
            return false;
    }

    for (int i = 0; i < KQT_AUDIO_UNITS_MAX; ++i)
    {
        const Audio_unit* sub_au = Au_table_get(au->au_table, i);
        if ((sub_au != NULL) &&
                !Device_set_buffer_size((const Device*)sub_au, dstates, size))
            return false;
    }

    return true;
}


static void mix_interface_connection(
        Device_state* out_ds,
        const Device_state* in_ds,
        uint32_t buf_start,
        uint32_t buf_stop)
{
    assert(out_ds != NULL);
    assert(in_ds != NULL);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Audio_buffer* out = Device_state_get_audio_buffer(
                out_ds, DEVICE_PORT_TYPE_SEND, port);
        const Audio_buffer* in = Device_state_get_audio_buffer(
                in_ds, DEVICE_PORT_TYPE_RECEIVE, port);

        if ((out != NULL) && (in != NULL))
            Audio_buffer_mix(out, in, buf_start, buf_stop);
    }

    return;
}


static void Audio_unit_process_signal(
        const Device* device,
        Device_states* dstates,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(audio_rate > 0);
    assert(isfinite(tempo));

    Au_state* au_state = (Au_state*)Device_states_get_state(
            dstates, Device_get_id(device));

    const Audio_unit* au = (const Audio_unit*)device;

    if (au_state->bypass)
    {
        Device_state* ds = Device_states_get_state(dstates, Device_get_id(device));

        mix_interface_connection(ds, ds, buf_start, buf_stop);
    }
    else if (au->connections != NULL)
    {
        //Connections_clear_buffers(au->connections, dstates, buf_start, buf_stop);

        // Fill input interface buffers
        Device_state* ds = Device_states_get_state(dstates, Device_get_id(device));
        Device_state* in_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Audio_unit_get_input_interface(au)));
        mix_interface_connection(in_iface_ds, ds, buf_start, buf_stop);

        // Process autrument graph
        Connections_mix(
                au->connections, dstates, buf_start, buf_stop, audio_rate, tempo);

        // Fill output interface buffers
        Device_state* out_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Audio_unit_get_output_interface(au)));
        mix_interface_connection(ds, out_iface_ds, buf_start, buf_stop);
    }

    return;
}


void del_Audio_unit(Audio_unit* au)
{
    if (au == NULL)
        return;

    Au_params_deinit(&au->params);
    del_Connections(au->connections);
    del_Au_interface(au->in_iface);
    del_Au_interface(au->out_iface);
    del_Au_table(au->au_table);
    del_Proc_table(au->procs);
    Device_deinit(&au->parent);
    memory_free(au);

    return;
}


