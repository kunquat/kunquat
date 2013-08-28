

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


#include <inttypes.h>
#include <math.h>
#include <stdlib.h>

#include <Decl.h>
#include <Device.h>
#include <Device_impl.h>
#include <math_common.h>
#include <string_common.h>
#include <xassert.h>


void Device_reset_default(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));
        Device_impl_reset_device_state(device->dimpl, dstate);
    }

    return;
}


bool Device_init(Device* device, uint32_t buffer_size, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    static uint32_t id = 1;
    device->id = id;
    ++id;

    device->existent = false;
    device->mix_rate = mix_rate;
    device->buffer_size = buffer_size;

    device->dparams = NULL;
    device->dimpl = NULL;

    device->create_state = new_Device_state_plain;
    device->set_mix_rate = NULL;
    device->set_buffer_size = NULL;
    device->reset = Device_reset_default;
    device->sync = NULL;
    device->update_key = NULL;
    device->process = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            device->reg[type][port] = false;
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

    // TODO: sync dimpl

    del_Device_impl(device->dimpl);
    device->dimpl = dimpl;

    return true;
}


Device_state* Device_create_state(const Device* device)
{
    assert(device != NULL);
    assert(device->create_state != NULL);

    return device->create_state(device, device->mix_rate, device->buffer_size);
}


void Device_set_state_creator(
        Device* device,
        Device_state* (*creator)(const Device*, int32_t, int32_t))
{
    assert(device != NULL);

    if (creator != NULL)
        device->create_state = creator;
    else
        device->create_state = new_Device_state_plain;

    return;
}


void Device_set_mix_rate_changer(
        Device* device,
        bool (*changer)(Device*, Device_states*, uint32_t))
{
    assert(device != NULL);
    device->set_mix_rate = changer;
    return;
}


void Device_set_buffer_size_changer(
        Device* device,
        bool (*changer)(Device*, Device_states*, uint32_t))
{
    assert(device != NULL);
    device->set_buffer_size = changer;
    return;
}


void Device_set_reset(Device* device, void (*reset)(Device*, Device_states*))
{
    assert(device != NULL);
    assert(reset != NULL);

    device->reset = reset;

    return;
}


void Device_set_sync(Device* device, bool (*sync)(Device*, Device_states*))
{
    assert(device != NULL);
    assert(sync != NULL);

    device->sync = sync;

    return;
}


void Device_set_update_key(
        Device* device,
        bool (*update_key)(struct Device*, const char*))
{
    assert(device != NULL);
    assert(update_key != NULL);
    device->update_key = update_key;
    return;
}


void Device_set_process(
        Device* device,
        void (*process)(
            Device*,
            Device_states*,
            uint32_t,
            uint32_t,
            uint32_t,
            double))
{
    assert(device != NULL);
    assert(process != NULL);
    device->process = process;
    return;
}


void Device_register_port(Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    device->reg[type][port] = true;

    return;
}


void Device_unregister_port(Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    device->reg[type][port] = false;

    return;
}


bool Device_port_is_registered(
        const Device* device,
        Device_port_type type,
        int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    return device->reg[type][port];
}


bool Device_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(rate > 0);

    if (device->set_mix_rate != NULL &&
            !device->set_mix_rate(device, dstates, rate))
        return false;

    device->mix_rate = rate;

    return true;
}


uint32_t Device_get_mix_rate(const Device* device)
{
    assert(device != NULL);
    return device->mix_rate;
}


bool Device_set_buffer_size(
        Device* device,
        Device_states* dstates,
        uint32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    if (device->set_buffer_size != NULL &&
            !device->set_buffer_size(device, dstates, size))
    {
        device->buffer_size = MIN(device->buffer_size, size);
        return false;
    }

    device->buffer_size = size;

    return true;
}


uint32_t Device_get_buffer_size(const Device* device)
{
    assert(device != NULL);
    return device->buffer_size;
}


void Device_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);

    if (device->reset != NULL)
        device->reset(device, dstates);

    return;
}


bool Device_sync(Device* device, Device_states* dstates)
{
    assert(device != NULL);

    if (device->sync != NULL)
        return device->sync(device, dstates);

    return true;
}


bool Device_set_key(
        Device* device,
        const char* key,
        void* data,
        long length,
        Read_state* rs)
{
    assert(device != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    assert(data != NULL || length == 0);
    assert(length >= 0);
    assert(rs != NULL);

    if (rs->error)
        return false;

    if (!Device_params_parse_value(device->dparams, key, data, length, rs))
        return false;

    if (device->dimpl != NULL)
        return Device_impl_set_key(device->dimpl, key + 2);

    return true;
}


void Device_notify_key_change(
        const Device* device,
        const char* key,
        Device_states* dstates)
{
    assert(device != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    assert(dstates != NULL);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));
        Device_impl_notify_key_change(device->dimpl, key + 2, dstate);
    }

    return;
}


void Device_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(start < device->buffer_size);
    assert(until <= device->buffer_size);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (device->process != NULL)
        device->process(device, states, start, until, freq, tempo);

    return;
}


void Device_print(Device* device, FILE* out)
{
    assert(device != NULL);
    assert(out != NULL);

    fprintf(out, "Device buffer size: %" PRIu32 " frames\n",
                 device->buffer_size);

    bool printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!device->reg[DEVICE_PORT_TYPE_SEND][port])
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
        if (!device->reg[DEVICE_PORT_TYPE_RECEIVE][port])
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
        device->reg[DEVICE_PORT_TYPE_RECEIVE][port] = false;
        device->reg[DEVICE_PORT_TYPE_SEND][port] = false;
    }

    return;
}


