

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include <Device.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


bool Device_init(Device* device, uint32_t buffer_size, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    device->mix_rate = mix_rate;
    device->buffer_size = buffer_size;
    device->set_mix_rate = NULL;
    device->set_buffer_size = NULL;
    device->reset = NULL;
    device->sync = NULL;
    device->process = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            device->reg[type][port] = false;
            device->buffers[type][port] = NULL;
        }
        device->direct_send[port] = NULL;
    }
    return true;
}


void Device_set_mix_rate_changer(Device* device,
                                 bool (*changer)(Device*, uint32_t))
{
    assert(device != NULL);
    device->set_mix_rate = changer;
    return;
}


void Device_set_buffer_size_changer(Device* device,
                                    bool (*changer)(Device*, uint32_t))
{
    assert(device != NULL);
    device->set_buffer_size = changer;
    return;
}


void Device_set_reset(Device* device, void (*reset)(Device*))
{
    assert(device != NULL);
    assert(reset != NULL);
    device->reset = reset;
    return;
}


void Device_set_sync(Device* device, bool (*sync)(Device*))
{
    assert(device != NULL);
    assert(sync != NULL);
    device->sync = sync;
    return;
}


void Device_set_process(Device* device,
                        void (*process)(Device*, uint32_t, uint32_t,
                                                 uint32_t, double))
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


bool Device_init_buffer(Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    if (!device->reg[type][port])
    {
        return true;
    }
    if (device->buffers[type][port] != NULL)
    {
        if (type == DEVICE_PORT_TYPE_SEND)
        {
            device->direct_send[port] = NULL;
        }
        return true;
    }
    Audio_buffer* buffer = new_Audio_buffer(device->buffer_size);
    if (buffer == NULL)
    {
        return false;
    }
//    fprintf(stderr, "Initialised buffer %p\n", (void*)buffer);
    assert(device->buffers[type][port] == NULL);
    device->buffers[type][port] = buffer;
    return true;
}


void Device_set_direct_receive(Device* device, int port)
{
    assert(device != NULL);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    assert(Device_get_buffer(device, DEVICE_PORT_TYPE_SEND, port) != NULL);
    if (device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] != NULL &&
            device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] !=
                device->buffers[DEVICE_PORT_TYPE_SEND][port] &&
            device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] !=
                device->direct_send[port])
    {
        del_Audio_buffer(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port]);
    }
    device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] =
            Device_get_buffer(device, DEVICE_PORT_TYPE_SEND, port);
//    fprintf(stderr, "Set direct receive %p in device %p\n",
//            (void*)device->buffers[DEVICE_PORT_TYPE_RECEIVE][port],
//            (void*)device);
    return;
}


void Device_set_direct_send(Device* device,
                            int port,
                            Audio_buffer* buffer)
{
    assert(device != NULL);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    assert(Audio_buffer_get_size(buffer) == device->buffer_size);
    device->direct_send[port] = buffer;
    return;
}


void Device_remove_direct_buffers(Device* device)
{
    assert(device != NULL);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        // If this is an Instrument Device, the input buffer of a port number
        // may be the same as the output with the same port number.
        Audio_buffer* in = device->buffers[DEVICE_PORT_TYPE_RECEIVE][port];
        if (in != NULL &&
                (in == device->buffers[DEVICE_PORT_TYPE_SEND][port] ||
                 in == device->direct_send[port]))
        {
            device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] = NULL;
        }
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        device->direct_send[port] = NULL;
    }
//    fprintf(stderr, "Removed direct buffers in %p \n", (void*)device);
    return;
}


bool Device_set_mix_rate(Device* device, uint32_t rate)
{
    assert(device != NULL);
    assert(rate > 0);
    if (device->set_mix_rate != NULL && !device->set_mix_rate(device, rate))
    {
        return false;
    }
    device->mix_rate = rate;
    return true;
}


uint32_t Device_get_mix_rate(Device* device)
{
    assert(device != NULL);
    return device->mix_rate;
}


bool Device_set_buffer_size(Device* device, uint32_t size)
{
    assert(device != NULL);
    assert(size > 0);
    assert(size <= KQT_BUFFER_SIZE_MAX);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            if (device->buffers[type][port] != NULL &&
                    !Audio_buffer_resize(device->buffers[type][port], size))
            {
                device->buffer_size = MIN(device->buffer_size, size);
                return false;
            }
        }
    }
    if (device->set_buffer_size != NULL &&
            !device->set_buffer_size(device, size))
    {
        device->buffer_size = MIN(device->buffer_size, size);
        return false;
    }
    device->buffer_size = size;
    return true;
}


uint32_t Device_get_buffer_size(Device* device)
{
    assert(device != NULL);
    return device->buffer_size;
}


Audio_buffer* Device_get_buffer(Device* device,
                                Device_port_type type,
                                int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    if (!device->reg[type][port])
    {
        return NULL;
    }
    if (type == DEVICE_PORT_TYPE_RECEIVE)
    {
        return device->buffers[type][port];
    }
    if (device->direct_send[port] != NULL)
    {
        return device->direct_send[port];
    }
    return device->buffers[type][port];
}


void Device_clear_buffers(Device* device, uint32_t start, uint32_t until)
{
    assert(device != NULL);
    //fprintf(stderr, "Accessing device %p\n", (void*)device);
    assert(start < device->buffer_size);
    assert(until <= device->buffer_size);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Audio_buffer* receive = device->buffers[DEVICE_PORT_TYPE_RECEIVE][port];
        Audio_buffer* send = device->buffers[DEVICE_PORT_TYPE_SEND][port];
        if (receive != NULL)
        {
            Audio_buffer_clear(receive, start, until);
        }
        if (send != NULL && send != receive)
        {
            Audio_buffer_clear(send, start, until);
        }
    }
    return;
}


void Device_reset(Device* device)
{
    assert(device != NULL);
    if (device->reset != NULL)
    {
        device->reset(device);
    }
    return;
}


bool Device_sync(Device* device)
{
    assert(device != NULL);
    if (device->sync != NULL)
    {
        return device->sync(device);
    }
    return true;
}


void Device_process(Device* device,
                    uint32_t start,
                    uint32_t until,
                    uint32_t freq,
                    double tempo)
{
    assert(device != NULL);
    assert(start < device->buffer_size);
    assert(until <= device->buffer_size);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);
    if (device->process != NULL)
    {
        device->process(device, start, until, freq, tempo);
    }
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
        {
            continue;
        }
        if (!printed)
        {
            fprintf(out, "Registered send ports:\n");
            printed = true;
        }
        bool direct = true;
        Audio_buffer* buffer = device->direct_send[port];
        if (buffer == NULL)
        {
            direct = false;
            buffer = device->buffers[DEVICE_PORT_TYPE_SEND][port];
        }
        fprintf(out, "  Port %02x, buffer %p%s\n", port, (void*)buffer,
                     direct ? " (direct)" : "");
    }
    printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!device->reg[DEVICE_PORT_TYPE_RECEIVE][port])
        {
            continue;
        }
        if (!printed)
        {
            fprintf(out, "Registered receive ports:\n");
            printed = true;
        }
        bool direct = device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] ==
                      Device_get_buffer(device, DEVICE_PORT_TYPE_SEND, port);
        fprintf(out, "  Port %02x, buffer %p%s\n", port,
                     (void*)device->buffers[DEVICE_PORT_TYPE_RECEIVE][port],
                     direct ? " (direct)" : "");
    }
    return;
}


void Device_uninit(Device* device)
{
    if (device == NULL)
    {
        return;
    }
    //fprintf(stderr, "Destroying device %p\n", (void*)device);
    Device_remove_direct_buffers(device);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        del_Audio_buffer(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port]);
        device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] = NULL;
        del_Audio_buffer(device->buffers[DEVICE_PORT_TYPE_SEND][port]);
        device->buffers[DEVICE_PORT_TYPE_SEND][port] = NULL;
        device->reg[DEVICE_PORT_TYPE_RECEIVE][port] = false;
        device->reg[DEVICE_PORT_TYPE_SEND][port] = false;
        device->direct_send[port] = NULL;
    }
    return;
}


