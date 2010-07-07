

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

#include <Device.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


bool Device_init(Device* device, uint32_t buffer_size)
{
    assert(device != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    device->buffer_size = buffer_size;
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


void Device_set_direct_send(Device* device,
                            int port,
                            Audio_buffer* buffer)
{
    assert(device != NULL);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    device->direct_send[port] = buffer;
    return;
}


bool Device_init_buffers(Device* device)
{
    assert(device != NULL);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Audio_buffer** receive = &device->buffers[DEVICE_PORT_TYPE_RECEIVE][port];
        Audio_buffer** send = &device->buffers[DEVICE_PORT_TYPE_SEND][port];
        if (device->reg[DEVICE_PORT_TYPE_RECEIVE][port])
        {
            if (*receive == NULL)
            {
                *receive = new_Audio_buffer(device->buffer_size);
                if (*receive == NULL)
                {
                    return false;
                }
            }
        }
        else
        {
            if (*receive != NULL)
            {
                del_Audio_buffer(*receive);
                *receive = NULL;
            }
        }
        if (device->reg[DEVICE_PORT_TYPE_SEND][port] &&
                device->direct_send[port] == NULL)
        {
            if (*send == NULL)
            {
                *send = new_Audio_buffer(device->buffer_size);
                if (*send == NULL)
                {
                    return false;
                }
            }
        }
        else
        {
            if (*send != NULL)
            {
                del_Audio_buffer(*send);
                *send = NULL;
            }
        }
    }
    return true;
}


bool Device_resize_buffers(Device* device, uint32_t size)
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
    device->buffer_size = size;
    return true;
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


void Device_clear_buffers(Device* device)
{
    assert(device != NULL);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] != NULL)
        {
            Audio_buffer_clear(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port]);
        }
        if (device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL)
        {
            Audio_buffer_clear(device->buffers[DEVICE_PORT_TYPE_SEND][port]);
        }
    }
    return;
}


void Device_uninit(Device* device)
{
    assert(device != NULL);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] != NULL)
        {
            del_Audio_buffer(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port]);
        }
        if (device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL)
        {
            del_Audio_buffer(device->buffers[DEVICE_PORT_TYPE_SEND][port]);
        }
        device->reg[DEVICE_PORT_TYPE_RECEIVE][port] = false;
        device->reg[DEVICE_PORT_TYPE_SEND][port] = false;
        device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] = NULL;
        device->buffers[DEVICE_PORT_TYPE_SEND][port] = NULL;
        device->direct_send[port] = NULL;
    }
    return;
}


