

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
#include <xassert.h>


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

    device->di = NULL;

    device->create_state = new_Device_state_plain;
    device->set_mix_rate = NULL;
    device->set_buffer_size = NULL;
    device->reset = NULL;
    device->sync = NULL;
    device->update_key = NULL;
    device->update_state_key = NULL;
    device->process = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            device->reg[type][port] = false;
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


void Device_set_update_state_key(
        Device* device,
        bool (*update_state_key)(Device*, Device_states*, const char*))
{
    assert(device != NULL);
    assert(update_state_key != NULL);
    device->update_state_key = update_state_key;
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


bool Device_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);

    // TODO: obsolete, remove
    if (device->update_key != NULL)
        return device->update_key(device, key);

    if (device->di != NULL)
        return Device_impl_update_key(device->di, key);

    return true;
}


bool Device_update_state_key(
        Device* device,
        Device_states* dstates,
        const char* key)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(key != NULL);

    if (device->update_state_key != NULL)
        return device->update_state_key(device, dstates, key);

    return true;
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

    del_Device_impl(device->di);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        device->reg[DEVICE_PORT_TYPE_RECEIVE][port] = false;
        device->reg[DEVICE_PORT_TYPE_SEND][port] = false;
    }

    return;
}


