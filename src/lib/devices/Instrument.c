

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
#include <devices/Device.h>
#include <devices/Effect_interface.h>
#include <devices/Proc_table.h>
#include <devices/Processor.h>
#include <devices/Instrument.h>
#include <memory.h>
#include <module/Ins_table.h>
#include <player/Ins_state.h>
#include <string/common.h>


struct Instrument
{
    Device parent;

    Effect_interface* out_iface;
    Effect_interface* in_iface;
    Connections* connections;

//    double default_force;       ///< Default force.

    int scale_index;            ///< The index of the Scale used (-1 means the default).

    Instrument_params params;   ///< All the Instrument parameters that Processors need.

    Proc_table* procs;
    Ins_table* insts;
};


static Device_state* Instrument_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void Instrument_reset(const Device* device, Device_states* dstates);

static bool Instrument_set_audio_rate(
        const Device* device,
        Device_states* dstates,
        int32_t mix_rate);

static bool Instrument_set_buffer_size(
        const Device* device,
        Device_states* dstates,
        int32_t size);

static void Instrument_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo);

//static bool Instrument_sync(Device* device, Device_states* dstates);

static void Instrument_process_signal(
        const Device* device,
        Device_states* dstates,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo);


Instrument* new_Instrument(void)
{
    Instrument* ins = memory_alloc_item(Instrument);
    if (ins == NULL)
        return NULL;

    //fprintf(stderr, "New Instrument %p\n", (void*)ins);
    ins->out_iface = NULL;
    ins->in_iface = NULL;
    ins->connections = NULL;
    ins->procs = NULL;

    if (!Device_init(&ins->parent, false))
    {
        memory_free(ins);
        return NULL;
    }
    if (Instrument_params_init(
                &ins->params,
                Device_get_id(&ins->parent)) == NULL)
    {
        Device_deinit(&ins->parent);
        memory_free(ins);
        return NULL;
    }

    Device_set_state_creator(&ins->parent, Instrument_create_state);
    Device_set_reset(&ins->parent, Instrument_reset);
    Device_register_set_audio_rate(&ins->parent, Instrument_set_audio_rate);
    Device_register_update_tempo(&ins->parent, Instrument_update_tempo);
    Device_register_set_buffer_size(&ins->parent, Instrument_set_buffer_size);
    Device_set_process(&ins->parent, Instrument_process_signal);

    ins->out_iface = new_Effect_interface();
    ins->in_iface = new_Effect_interface();
    ins->procs = new_Proc_table(KQT_PROCESSORS_MAX);
    ins->insts = new_Ins_table(KQT_INSTRUMENTS_MAX);
    if ((ins->out_iface == NULL) ||
            (ins->in_iface == NULL) ||
            (ins->procs == NULL) ||
            (ins->insts == NULL))
    {
        del_Instrument(ins);
        return NULL;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_set_port_existence(
                &ins->out_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
        Device_set_port_existence(
                &ins->in_iface->parent, DEVICE_PORT_TYPE_SEND, port, true);
    }
    Device_set_port_existence(
            &ins->out_iface->parent, DEVICE_PORT_TYPE_RECEIVE, 0, true);

//    ins->default_force = INS_DEFAULT_FORCE;
    ins->params.force_variation = INS_DEFAULT_FORCE_VAR;

    ins->scale_index = INS_DEFAULT_SCALE_INDEX;

    return ins;
}


typedef struct ins_params
{
    double global_force;
    double default_force;
    double force_variation;
    int64_t scale_index;
} ins_params;

static bool read_ins_field(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    ins_params* p = userdata;

    if (string_eq(key, "force"))
        Streader_read_float(sr, &p->default_force);
    else if (string_eq(key, "force_variation"))
        Streader_read_float(sr, &p->force_variation);
    else if (string_eq(key, "global_force"))
        Streader_read_float(sr, &p->global_force);
#if 0
    else if (string_eq(key, "pitch_lock"))
        str = read_bool(str, &pitch_lock_enabled, state);
    else if (string_eq(key, "pitch_lock_cents"))
        str = read_double(str, &pitch_lock_cents, state);
#endif
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
                sr, "Unsupported field in instrument information: %s", key);
        return false;
    }

    return !Streader_is_error_set(sr);
}

bool Instrument_parse_header(Instrument* ins, Streader* sr)
{
    assert(ins != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    ins_params* p = &(ins_params)
    {
        .global_force = INS_DEFAULT_GLOBAL_FORCE,
        .default_force = INS_DEFAULT_FORCE,
        .force_variation = INS_DEFAULT_FORCE_VAR,
#if 0
        bool pitch_lock_enabled = false;
        double pitch_lock_cents = 0;
#endif
        .scale_index = INS_DEFAULT_SCALE_INDEX,
    };

    if (Streader_has_data(sr) && !Streader_read_dict(sr, read_ins_field, p))
        return false;

    ins->params.force = p->default_force;
    ins->params.force_variation = p->force_variation;
    ins->params.global_force = exp2(p->global_force / 6);
#if 0
    ins->params.pitch_lock_enabled = pitch_lock_enabled;
    ins->params.pitch_lock_cents = pitch_lock_cents;
    ins->params.pitch_lock_freq = exp2(ins->params.pitch_lock_cents / 1200.0) * 440;
#endif

    return true;
}


bool Instrument_parse_value(Instrument* ins, const char* subkey, Streader* sr)
{
    assert(ins != NULL);
    assert(subkey != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    int proc_index = -1;
    if ((proc_index = string_extract_index(subkey,
                             "p_pitch_lock_enabled_", 2, ".json")) >= 0 &&
            proc_index < KQT_PROCESSORS_MAX)
    {
        if (!Streader_read_bool(sr, &ins->params.pitch_locks[proc_index].enabled))
            return false;
    }
    else if ((proc_index = string_extract_index(subkey,
                                  "p_pitch_lock_cents_", 2, ".json")) >= 0 &&
            proc_index < KQT_PROCESSORS_MAX)
    {
        if (!Streader_read_float(sr, &ins->params.pitch_locks[proc_index].cents))
            return false;

        ins->params.pitch_locks[proc_index].freq =
                exp2(ins->params.pitch_locks[proc_index].cents / 1200.0) * 440;
    }

    return true;
}


Instrument_params* Instrument_get_params(Instrument* ins)
{
    assert(ins != NULL);
    return &ins->params;
}


const Processor* Instrument_get_proc(const Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(index >= 0);
    assert(index < KQT_PROCESSORS_MAX);

    return Proc_table_get_proc(ins->procs, index);
}


Proc_table* Instrument_get_procs(Instrument* ins)
{
    assert(ins != NULL);
    return ins->procs;
}


const Instrument* Instrument_get_ins(const Instrument* ins, int index)
{
    assert(ins != NULL);
    assert(ins->insts != NULL);
    assert(index >= 0);
    assert(index < KQT_INSTRUMENTS_MAX);

    return Ins_table_get(ins->insts, index);
}


Ins_table* Instrument_get_insts(Instrument* ins)
{
    assert(ins != NULL);
    assert(ins->insts != NULL);

    return ins->insts;
}


void Instrument_set_connections(Instrument* ins, Connections* graph)
{
    assert(ins != NULL);

    if (ins->connections != NULL)
        del_Connections(ins->connections);
    ins->connections = graph;

    return;
}


const Connections* Instrument_get_connections(const Instrument* ins)
{
    assert(ins != NULL);
    return ins->connections;
}


Connections* Instrument_get_connections_mut(const Instrument* ins)
{
    assert(ins != NULL);
    return ins->connections;
}


bool Instrument_prepare_connections(const Instrument* ins, Device_states* states)
{
    assert(ins != NULL);
    assert(states != NULL);

    if (ins->connections == NULL)
        return true;

    return Connections_prepare(ins->connections, states);
}


const Device* Instrument_get_input_interface(const Instrument* ins)
{
    assert(ins != NULL);
    return &ins->in_iface->parent;
}


const Device* Instrument_get_output_interface(const Instrument* ins)
{
    assert(ins != NULL);
    return &ins->out_iface->parent;
}


static Device_state* Instrument_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Ins_state* is = memory_alloc_item(Ins_state);
    if (is == NULL)
        return NULL;

    Device_state_init(&is->parent, device, audio_rate, audio_buffer_size);
    Ins_state_reset(is);

    return &is->parent;
}


static void Instrument_reset(const Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    const Instrument* ins = (const Instrument*)device;

    // Reset processors
    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(ins->procs, i);
        if (proc != NULL)
            Device_reset((const Device*)proc, dstates);
    }

    // Reset internal instruments
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        const Instrument* sub_ins = Ins_table_get(ins->insts, i);
        if (sub_ins != NULL)
            Device_reset((const Device*)sub_ins, dstates);
    }

    // Reset instrument state
    Ins_state* ins_state = (Ins_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));
    Ins_state_reset(ins_state);

    return;
}


static bool Instrument_set_audio_rate(
        const Device* device,
        Device_states* dstates,
        int32_t audio_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(audio_rate > 0);

    const Instrument* ins = (const Instrument*)device;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(ins->procs, i);
        if (proc != NULL &&
                !Device_set_audio_rate((const Device*)proc, dstates, audio_rate))
            return false;
    }

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        const Instrument* sub_ins = Ins_table_get(ins->insts, i);
        if ((sub_ins != NULL) &&
                !Device_set_audio_rate((const Device*)sub_ins, dstates, audio_rate))
            return false;
    }

    return true;
}


static void Instrument_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    const Instrument* ins = (const Instrument*)device;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(ins->procs, i);
        if (proc != NULL)
            Device_update_tempo((const Device*)proc, dstates, tempo);
    }

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        const Instrument* sub_ins = Ins_table_get(ins->insts, i);
        if (sub_ins != NULL)
            Device_update_tempo((const Device*)sub_ins, dstates, tempo);
    }

    return;
}


static bool Instrument_set_buffer_size(
        const Device* device,
        Device_states* dstates,
        int32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    const Instrument* ins = (const Instrument*)device;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Proc_table_get_proc(ins->procs, i);
        if (proc != NULL &&
                !Device_set_buffer_size((const Device*)proc, dstates, size))
            return false;
    }

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        const Instrument* sub_ins = Ins_table_get(ins->insts, i);
        if ((sub_ins != NULL) &&
                !Device_set_buffer_size((const Device*)sub_ins, dstates, size))
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


static void Instrument_process_signal(
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

    Ins_state* ins_state = (Ins_state*)Device_states_get_state(
            dstates, Device_get_id(device));

    const Instrument* ins = (const Instrument*)device;

    if (ins_state->bypass)
    {
        Device_state* ds = Device_states_get_state(dstates, Device_get_id(device));

        mix_interface_connection(ds, ds, buf_start, buf_stop);
    }
    else if (ins->connections != NULL)
    {
        //Connections_clear_buffers(ins->connections, dstates, buf_start, buf_stop);

        // Fill input interface buffers
        Device_state* ds = Device_states_get_state(dstates, Device_get_id(device));
        Device_state* in_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Instrument_get_input_interface(ins)));
        mix_interface_connection(in_iface_ds, ds, buf_start, buf_stop);

        // Process instrument graph
        Connections_mix(
                ins->connections, dstates, buf_start, buf_stop, audio_rate, tempo);

        // Fill output interface buffers
        Device_state* out_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Instrument_get_output_interface(ins)));
        mix_interface_connection(ds, out_iface_ds, buf_start, buf_stop);
    }

    return;
}


void del_Instrument(Instrument* ins)
{
    if (ins == NULL)
        return;

    Instrument_params_deinit(&ins->params);
    del_Connections(ins->connections);
    del_Effect_interface(ins->in_iface);
    del_Effect_interface(ins->out_iface);
    del_Proc_table(ins->procs);
    Device_deinit(&ins->parent);
    memory_free(ins);

    return;
}


