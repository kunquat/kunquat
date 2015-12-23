

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


#include <devices/Audio_unit.h>

#include <Connections.h>
#include <debug/assert.h>
#include <devices/Au_control_vars.h>
#include <devices/Au_interface.h>
#include <devices/Device.h>
#include <devices/Proc_table.h>
#include <devices/Processor.h>
#include <mathnum/common.h>
#include <mathnum/Random.h>
#include <memory.h>
#include <module/Au_table.h>
#include <player/Channel.h>
#include <player/devices/Au_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

    Au_control_vars* control_vars;
};


static Device_state* Audio_unit_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Audio_unit_update_tempo(
        const Device* device, Device_states* dstates, double tempo);

//static bool Audio_unit_sync(Device* device, Device_states* dstates);

static void Audio_unit_process_signal(
        const Device* device,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo);

static void Audio_unit_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value);

static void Audio_unit_slide_control_var_float_target(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double value);

static void Audio_unit_slide_control_var_float_length(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length);

static void Audio_unit_osc_speed_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double speed);

static void Audio_unit_osc_depth_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double depth);

static void Audio_unit_osc_speed_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length);

static void Audio_unit_osc_depth_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length);

static void Audio_unit_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel);

static void Audio_unit_init_control_var_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Linear_controls* controls);


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
    au->control_vars = NULL;

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
    Device_register_update_tempo(&au->parent, Audio_unit_update_tempo);
    Device_set_mixed_signals(&au->parent, true);
    Device_set_process(&au->parent, Audio_unit_process_signal);

    Device_register_set_control_var_generic(
            &au->parent, Audio_unit_set_control_var_generic);
    Device_register_slide_control_var_float(
            &au->parent,
            Audio_unit_slide_control_var_float_target,
            Audio_unit_slide_control_var_float_length);
    Device_register_osc_cv_float(
            &au->parent,
            Audio_unit_osc_speed_cv_float,
            Audio_unit_osc_depth_cv_float,
            Audio_unit_osc_speed_slide_cv_float,
            Audio_unit_osc_depth_slide_cv_float);

    Device_register_init_control_vars(&au->parent, Audio_unit_init_control_vars);
    Device_register_init_control_var_float(
            &au->parent, Audio_unit_init_control_var_float);

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
    double global_lowpass;
    double default_lowpass;
    double default_resonance;
    double pitch_lowpass_scale;
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
    else if (string_eq(key, "global_lowpass"))
        Streader_read_float(sr, &p->global_lowpass);
    else if (string_eq(key, "default_lowpass"))
        Streader_read_float(sr, &p->default_lowpass);
    else if (string_eq(key, "default_resonance"))
        Streader_read_float(sr, &p->default_resonance);
    else if (string_eq(key, "pitch_lowpass_scale"))
        Streader_read_float(sr, &p->pitch_lowpass_scale);
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
                sr, "Unsupported field in audio unit information: %s", key);
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
        .global_lowpass = AU_DEFAULT_GLOBAL_LOWPASS,
        .default_lowpass = 100,
        .scale_index = AU_DEFAULT_SCALE_INDEX,
    };

    if (Streader_has_data(sr) && !Streader_read_dict(sr, read_au_field, p))
        return false;

    au->params.force = p->default_force;
    au->params.force_variation = p->force_variation;
    au->params.global_force = exp2(p->global_force / 6);
    au->params.global_lowpass = clamp(p->global_lowpass, 0, 200);
    au->params.default_lowpass = clamp(p->default_lowpass, 0, 100);
    au->params.default_resonance = clamp(p->default_resonance, 0, 100);
    au->params.pitch_lowpass_scale = clamp(p->pitch_lowpass_scale, -4, 4);

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


Proc_table* Audio_unit_get_procs(const Audio_unit* au)
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


void Audio_unit_set_control_vars(Audio_unit* au, Au_control_vars* au_control_vars)
{
    assert(au != NULL);

    del_Au_control_vars(au->control_vars);
    au->control_vars = au_control_vars;

    return;
}


static Device_state* Audio_unit_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Au_state* au_state = memory_alloc_item(Au_state);
    if (au_state == NULL)
        return NULL;

    Au_state_init(au_state, device, audio_rate, audio_buffer_size);

    return &au_state->parent;
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
        const Work_buffers* wbs,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
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

        // Process audio unit graph
        Connections_mix(
                au->connections, dstates, wbs, buf_start, buf_stop, audio_rate, tempo);

        // Fill output interface buffers
        Device_state* out_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Audio_unit_get_output_interface(au)));
        mix_interface_connection(ds, out_iface_ds, buf_start, buf_stop);
    }

    return;
}


static const Device* get_cv_target_device(
        const Audio_unit* au,
        const Au_control_binding_iter* iter,
        Device_control_var_mode mode)
{
    assert(au != NULL);
    assert(iter != NULL);

    const Device* target_dev = NULL;
    if ((mode == DEVICE_CONTROL_VAR_MODE_MIXED) &&
            (iter->target_dev_type == TARGET_DEV_AU))
        target_dev = (const Device*)Au_table_get(
                au->au_table, iter->target_dev_index);
    else if (iter->target_dev_type == TARGET_DEV_PROC)
        target_dev = (const Device*)Proc_table_get_proc(
                au->procs, iter->target_dev_index);

    return target_dev;
}


static void Audio_unit_init_control_var_generic(
        const Audio_unit* au,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        Value_type var_type)
{
    assert(au != NULL);
    assert(dstates != NULL);
    assert(random != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));
    assert(var_name != NULL);

    bool carried = false;

    if (mode == DEVICE_CONTROL_VAR_MODE_VOICE)
    {
        const Channel_cv_state* cvstate = Channel_get_cv_state(channel);
        if (Channel_cv_state_is_carrying_enabled(cvstate, var_name, var_type))
        {
            const Value* carried_value =
                Channel_cv_state_get_value(cvstate, var_name, var_type);
            if (carried_value != NULL)
            {
                Audio_unit_set_control_var_generic(
                        &au->parent,
                        dstates,
                        mode,
                        random,
                        channel,
                        var_name,
                        carried_value);

                carried = true;
            }
        }
    }

    if (!carried)
    {
        const Value* init_value =
            Au_control_vars_get_init_value(au->control_vars, var_name);

        if (mode == DEVICE_CONTROL_VAR_MODE_VOICE)
        {
            Channel_cv_state* cvstate = Channel_get_cv_state_mut(channel);
            const bool success =
                Channel_cv_state_set_value(cvstate, var_name, init_value);
            assert(success);
        }

        Audio_unit_set_control_var_generic(
                &au->parent, dstates, mode, random, channel, var_name, init_value);
    }

    return;
}


static void Audio_unit_init_control_var_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Linear_controls* controls)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(controls != NULL);

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Map controls to each bound target and
    // call initialiser for corresponding subdevice
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init_float_controls(
            iter, au->control_vars, var_name, controls);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if ((target_dev != NULL) && (iter->target_value.type == VALUE_TYPE_FLOAT))
            Device_init_control_var_float(
                    target_dev,
                    dstates,
                    mode,
                    channel,
                    iter->target_var_name,
                    &iter->target_controls);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_init_control_var_float_slide(
        const Audio_unit* au,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name)
{
    assert(au != NULL);
    assert(dstates != NULL);
    assert(random != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));
    assert(var_name != NULL);

    bool carried = false;

    if (mode == DEVICE_CONTROL_VAR_MODE_VOICE)
    {
        const Channel_cv_state* cvstate = Channel_get_cv_state(channel);
        if (Channel_cv_state_is_carrying_enabled(cvstate, var_name, VALUE_TYPE_FLOAT))
        {
            const Linear_controls* controls =
                Channel_cv_state_get_float_controls(cvstate, var_name);
            if (controls != NULL)
            {
                Device_init_control_var_float(
                        &au->parent, dstates, mode, channel, var_name, controls);
                carried = true;
            }
        }
    }

    if (!carried)
    {
        const Value* init_value =
            Au_control_vars_get_init_value(au->control_vars, var_name);
        assert(init_value->type == VALUE_TYPE_FLOAT);

        if (mode == DEVICE_CONTROL_VAR_MODE_VOICE)
        {
            Channel_cv_state* cvstate = Channel_get_cv_state_mut(channel);
            const bool success =
                Channel_cv_state_set_value(cvstate, var_name, init_value);
            assert(success);
        }

        Audio_unit_set_control_var_generic(
                &au->parent, dstates, mode, random, channel, var_name, init_value);
    }

    return;
}


static void Audio_unit_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(random != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    Au_control_var_iter* var_iter =
        Au_control_var_iter_init(AU_CONTROL_VAR_ITER_AUTO, au->control_vars);
    const char* var_name = NULL;
    Value_type var_type = VALUE_TYPE_NONE;
    Au_control_var_iter_get_next_var_info(var_iter, &var_name, &var_type);
    while (var_name != NULL)
    {
        if (Au_control_vars_is_entry_float_slide(au->control_vars, var_name))
            Audio_unit_init_control_var_float_slide(
                    au, dstates, mode, random, channel, var_name);
        else
            Audio_unit_init_control_var_generic(
                    au, dstates, mode, random, channel, var_name, var_type);

        Au_control_var_iter_get_next_var_info(var_iter, &var_name, &var_type);
    }

    return;
}


static void Audio_unit_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(random != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));
    assert(var_name != NULL);
    assert(value != NULL);

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Map our value to each bound target range and
    // call control value setter for corresponding subdevice
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init_set_generic(
            iter, au->control_vars, random, var_name, value);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if ((target_dev != NULL) && (iter->target_value.type != VALUE_TYPE_NONE))
            Device_set_control_var_generic(
                    target_dev,
                    dstates,
                    mode,
                    random,
                    channel,
                    iter->target_var_name,
                    &iter->target_value);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_slide_control_var_float_target(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double value)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(isfinite(value));

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    Value* wrapped = VALUE_AUTO;
    wrapped->type = VALUE_TYPE_FLOAT;
    wrapped->value.float_type = value;

    // Map our value to each bound target range and
    // call control value slider for corresponding subdevice
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init_set_generic(
            iter, au->control_vars, NULL, var_name, wrapped);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if ((target_dev != NULL) && (iter->target_value.type == VALUE_TYPE_FLOAT))
            Device_slide_control_var_float_target(
                    target_dev,
                    dstates,
                    mode,
                    channel,
                    iter->target_var_name,
                    iter->target_value.value.float_type);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_slide_control_var_float_length(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Set slide length for target devices
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init(iter, au->control_vars, var_name);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if (target_dev != NULL)
            Device_slide_control_var_float_length(
                    target_dev, dstates, mode, channel, iter->target_var_name, length);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_osc_speed_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double speed)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(speed >= 0);

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Set oscillation speed for target devices
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init(iter, au->control_vars, var_name);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if (target_dev != NULL)
            Device_osc_speed_cv_float(
                    target_dev, dstates, mode, channel, iter->target_var_name, speed);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_osc_depth_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double depth)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(isfinite(depth));

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Set oscillation depth for target devices
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init_osc_depth_float(
            iter, au->control_vars, var_name, depth);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if ((target_dev != NULL) && (iter->target_value.type == VALUE_TYPE_FLOAT))
            Device_osc_depth_cv_float(
                    target_dev,
                    dstates,
                    mode,
                    channel,
                    iter->target_var_name,
                    iter->target_value.value.float_type);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_osc_speed_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Audio_unit* au = (const Audio_unit*)device;
    if (au->control_vars == NULL)
        return;

    // Set oscillation speed slide for target devices
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init(iter, au->control_vars, var_name);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if (target_dev != NULL)
            Device_osc_speed_slide_cv_float(
                    target_dev, dstates, mode, channel, iter->target_var_name, length);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
    }

    return;
}


static void Audio_unit_osc_depth_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Audio_unit* au = (const Audio_unit*)device;

    if (au->control_vars == NULL)
        return;

    // Set oscillation depth slide for target devices
    Au_control_binding_iter* iter = AU_CONTROL_BINDING_ITER_AUTO;
    bool has_entry = Au_control_binding_iter_init(iter, au->control_vars, var_name);
    while (has_entry)
    {
        const Device* target_dev = get_cv_target_device(au, iter, mode);

        if (target_dev != NULL)
            Device_osc_depth_slide_cv_float(
                    target_dev, dstates, mode, channel, iter->target_var_name, length);

        has_entry = Au_control_binding_iter_get_next_entry(iter);
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
    del_Au_control_vars(au->control_vars);
    Device_deinit(&au->parent);
    memory_free(au);

    return;
}


