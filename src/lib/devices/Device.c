

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


#include <devices/Device.h>

#include <debug/assert.h>
#include <Decl.h>
#include <devices/Device_impl.h>
#include <mathnum/common.h>
#include <string/common.h>

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>


static bool Device_set_audio_rate_default(
        const Device* device, Device_states* dstates, int32_t audio_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(audio_rate > 0);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));
        return Device_impl_set_audio_rate(device->dimpl, dstate, audio_rate);
    }

    return true;
}


static bool Device_set_buffer_size_default(
        const Device* device, Device_states* dstates, int32_t buffer_size)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(buffer_size >= 0);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));
        return Device_impl_set_buffer_size(device->dimpl, dstate, buffer_size);
    }

    return true;
}


static void Device_update_tempo_default(
        const Device* device, Device_states* dstates, double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));
        Device_impl_update_tempo(device->dimpl, dstate, tempo);
    }

    return;
}


bool Device_init(Device* device, bool req_impl)
{
    assert(device != NULL);

    static uint32_t id = 1;
    device->id = id;
    ++id;

    device->existent = false;
    device->req_impl = req_impl;

    device->enable_signal_support = false;

    device->dparams = NULL;
    device->dimpl = NULL;

    device->create_state = new_Device_state_plain;
    device->set_audio_rate = Device_set_audio_rate_default;
    device->set_buffer_size = Device_set_buffer_size_default;
    device->update_tempo = Device_update_tempo_default;
    device->process_signal = NULL;

    device->set_control_var_generic = NULL;
    device->slide_control_var_float_target = NULL;
    device->slide_control_var_float_length = NULL;
    device->osc_speed_cv_float = NULL;
    device->osc_depth_cv_float = NULL;
    device->osc_speed_slide_cv_float = NULL;
    device->osc_depth_slide_cv_float = NULL;

    device->init_control_vars = NULL;
    device->init_control_var_float = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            device->existence[type][port] = false;
    }

    device->dparams = new_Device_params();
    if (device->dparams == NULL)
    {
        Device_deinit(device);
        return false;
    }

    return true;
}


uint32_t Device_get_id(const Device* device)
{
    assert(device != NULL);
    return device->id;
}


bool Device_has_complete_type(const Device* device)
{
    assert(device != NULL);
    return !device->req_impl || (device->dimpl != NULL);
}


void Device_set_existent(Device* device, bool existent)
{
    assert(device != NULL);
    device->existent = existent;
    return;
}


bool Device_is_existent(const Device* device)
{
    assert(device != NULL);
    return device->existent;
}


bool Device_set_impl(Device* device, Device_impl* dimpl)
{
    assert(device != NULL);
    assert(dimpl != NULL);

    Device_impl* old_dimpl = device->dimpl;
    device->dimpl = dimpl;

    if (!Device_impl_init(device->dimpl))
    {
        device->dimpl = old_dimpl;
        return false;
    }

    del_Device_impl(old_dimpl);

    return true;
}


Device_state* Device_create_state(
        const Device* device, int32_t audio_rate, int32_t buffer_size)
{
    assert(device != NULL);
    assert(device->create_state != NULL);
    assert(audio_rate > 0);
    assert(buffer_size >= 0);

    return device->create_state(device, audio_rate, buffer_size);
}


void Device_set_state_creator(
        Device* device, Device_state* (*creator)(const Device*, int32_t, int32_t))
{
    assert(device != NULL);

    if (creator != NULL)
        device->create_state = creator;
    else
        device->create_state = new_Device_state_plain;

    return;
}


void Device_register_set_audio_rate(
        Device* device, bool (*set)(const Device*, Device_states*, int32_t))
{
    assert(device != NULL);
    assert(set != NULL);

    device->set_audio_rate = set;

    return;
}


void Device_register_set_buffer_size(
        Device* device, bool (*set)(const Device*, Device_states*, int32_t))
{
    assert(device != NULL);
    assert(set != NULL);

    device->set_buffer_size = set;

    return;
}


void Device_register_update_tempo(
        Device* device, void (*update)(const Device*, Device_states*, double))
{
    assert(device != NULL);
    assert(update != NULL);

    device->update_tempo = update;

    return;
}


void Device_set_mixed_signals(Device* device, bool enabled)
{
    assert(device != NULL);
    device->enable_signal_support = enabled;
    return;
}


bool Device_get_mixed_signals(const Device* device)
{
    assert(device != NULL);
    return device->enable_signal_support;
}


void Device_set_process(Device* device, Device_process_signal_func* process_signal)
{
    assert(device != NULL);
    device->process_signal = process_signal;
    return;
}


void Device_register_set_control_var_generic(
        Device* device, Device_set_control_var_generic_func* set_func)
{
    assert(device != NULL);
    assert(set_func != NULL);

    device->set_control_var_generic = set_func;

    return;
}


void Device_register_slide_control_var_float(
        Device* device,
        Device_slide_control_var_float_target_func* slide_target_func,
        Device_slide_control_var_float_length_func* slide_length_func)
{
    assert(device != NULL);
    assert(slide_target_func != NULL);
    assert(slide_length_func != NULL);

    device->slide_control_var_float_target = slide_target_func;
    device->slide_control_var_float_length = slide_length_func;

    return;
}


void Device_register_osc_cv_float(
        Device* device,
        Device_osc_speed_cv_float_func* speed_func,
        Device_osc_depth_cv_float_func* depth_func,
        Device_osc_speed_slide_cv_float_func* speed_slide_func,
        Device_osc_depth_slide_cv_float_func* depth_slide_func)
{
    assert(device != NULL);
    assert(speed_func != NULL);
    assert(depth_func != NULL);
    assert(speed_slide_func != NULL);
    assert(depth_slide_func != NULL);

    device->osc_speed_cv_float = speed_func;
    device->osc_depth_cv_float = depth_func;
    device->osc_speed_slide_cv_float = speed_slide_func;
    device->osc_depth_slide_cv_float = depth_slide_func;

    return;
}


void Device_register_init_control_vars(
        Device* device, Device_init_control_vars_func* init_func)
{
    assert(device != NULL);
    assert(init_func != NULL);

    device->init_control_vars = init_func;

    return;
}


void Device_register_init_control_var_float(
        Device* device, Device_init_control_var_float_func* init_func)
{
    assert(device != NULL);
    assert(init_func != NULL);

    device->init_control_var_float = init_func;

    return;
}


void Device_set_port_existence(
        Device* device, Device_port_type type, int port, bool exists)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    device->existence[type][port] = exists;

    return;
}


bool Device_get_port_existence(const Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    return device->existence[type][port];
}


bool Device_set_audio_rate(const Device* device, Device_states* dstates, int32_t rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(rate > 0);

    assert(device->set_audio_rate != NULL);
    return device->set_audio_rate(device, dstates, rate);
}


bool Device_set_buffer_size(const Device* device, Device_states* dstates, int32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    assert(device->set_buffer_size != NULL);
    return device->set_buffer_size(device, dstates, size);
}


void Device_update_tempo(const Device* device, Device_states* dstates, double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    assert(device->update_tempo != NULL);
    device->update_tempo(device, dstates, tempo);

    return;
}


bool Device_sync(Device* device)
{
    assert(device != NULL);

    // Set existing keys on dimpl
    if (device->dimpl != NULL)
    {
        Device_params_iter* iter = Device_params_iter_init(
                DEVICE_PARAMS_ITER_AUTO, device->dparams);

        const char* key = Device_params_iter_get_next_key(iter);
        while (key != NULL)
        {
            if (!Device_impl_set_key(device->dimpl, key))
                return false;

            key = Device_params_iter_get_next_key(iter);
        }
    }

    return true;
}


bool Device_sync_states(const Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));

        Device_params_iter* iter = Device_params_iter_init(
                DEVICE_PARAMS_ITER_AUTO, device->dparams);

        const char* key = Device_params_iter_get_next_key(iter);
        while (key != NULL)
        {
            if (!Device_impl_set_state_key(device->dimpl, dstate, key))
                return false;

            key = Device_params_iter_get_next_key(iter);
        }
    }

    return true;
}


bool Device_set_key(Device* device, const char* key, Streader* sr)
{
    assert(device != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Device_params_parse_value(device->dparams, key, sr))
        return false;

    if (device->dimpl != NULL && !Device_impl_set_key(device->dimpl, key + 2))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for device key %s", key);
        return false;
    }

    return true;
}


bool Device_set_state_key(
        const Device* device,
        Device_states* dstates,
        const char* key)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));
        return Device_impl_set_state_key(device->dimpl, dstate, key + 2);
    }

    return true;
}


void Device_process(
        const Device* device,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (device->enable_signal_support && (device->process_signal != NULL))
        device->process_signal(device, states, wbs, start, until, freq, tempo);

    return;
}


void Device_set_control_var_generic(
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
    assert((value->type == VALUE_TYPE_BOOL) ||
            (value->type == VALUE_TYPE_INT) ||
            (value->type == VALUE_TYPE_FLOAT) ||
            (value->type == VALUE_TYPE_TSTAMP));

    if (device->set_control_var_generic != NULL)
        device->set_control_var_generic(
                device, dstates, mode, random, channel, var_name, value);

    return;
}


void Device_slide_control_var_float_target(
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

    if (device->slide_control_var_float_target != NULL)
        device->slide_control_var_float_target(
                device, dstates, mode, channel, var_name, value);

    return;
}


void Device_slide_control_var_float_length(
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

    if (device->slide_control_var_float_length != NULL)
        device->slide_control_var_float_length(
                device, dstates, mode, channel, var_name, length);

    return;
}


void Device_osc_speed_cv_float(
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
    assert(isfinite(speed));

    if (device->osc_speed_cv_float != NULL)
        device->osc_speed_cv_float(device, dstates, mode, channel, var_name, speed);

    return;
}


void Device_osc_depth_cv_float(
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

    if (device->osc_depth_cv_float != NULL)
        device->osc_depth_cv_float(device, dstates, mode, channel, var_name, depth);

    return;
}


void Device_osc_speed_slide_cv_float(
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

    if (device->osc_speed_slide_cv_float != NULL)
        device->osc_speed_slide_cv_float(
                device, dstates, mode, channel, var_name, length);

    return;
}


void Device_osc_depth_slide_cv_float(
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

    if (device->osc_depth_slide_cv_float != NULL)
        device->osc_depth_slide_cv_float(
                device, dstates, mode, channel, var_name, length);

    return;
}


void Device_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_MIXED, random != NULL));
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));

    if (device->init_control_vars != NULL)
        device->init_control_vars(device, dstates, mode, random, channel);

    return;
}


void Device_init_control_var_float(
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

    if (device->init_control_var_float != NULL)
        device->init_control_var_float(
                device, dstates, mode, channel, var_name, controls);

    return;
}


void Device_print(const Device* device, FILE* out)
{
    assert(device != NULL);
    assert(out != NULL);

    bool printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!device->existence[DEVICE_PORT_TYPE_SEND][port])
            continue;

        if (!printed)
        {
            fprintf(out, "Registered send ports:\n");
            printed = true;
        }

        fprintf(out, "  Port %02x\n", port);
    }

    printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!device->existence[DEVICE_PORT_TYPE_RECEIVE][port])
            continue;

        if (!printed)
        {
            fprintf(out, "Registered receive ports:\n");
            printed = true;
        }

        fprintf(out, "  Port %02x\n", port);
    }

    return;
}


void Device_deinit(Device* device)
{
    if (device == NULL)
        return;

    del_Device_impl(device->dimpl);
    device->dimpl = NULL;

    del_Device_params(device->dparams);
    device->dparams = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        device->existence[DEVICE_PORT_TYPE_RECEIVE][port] = false;
        device->existence[DEVICE_PORT_TYPE_SEND][port] = false;
    }

    return;
}


