

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
#include <assert.h>

#include <Device.h>
#include <math_common.h>

#include <xmemory.h>


bool Device_init(Device* device, uint32_t buffer_size)
{
    assert(device != NULL);
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    device->buffer_size = buffer_size;
    device->node = NULL;
    for (int i = 0; i < KQT_DEVICE_PORTS_MAX; ++i)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            device->reg[type][i] = false;
            device->buffers[type][i] = NULL;
        }
        device->direct_send[i] = NULL;
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
        if (device->reg[DEVICE_PORT_TYPE_RECEIVE][port])
        {
            assert(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] != NULL);
            if (!Audio_buffer_resize(
                        device->buffers[DEVICE_PORT_TYPE_RECEIVE][port], size))
            {
                device->buffer_size = MIN(device->buffer_size, size);
                return false;
            }
        }
        else
        {
            assert(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] == NULL);
        }
        if (device->reg[DEVICE_PORT_TYPE_SEND][port])
        {
            assert(device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL ||
                   device->direct_send[port] != NULL);
            if (device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL &&
                    !Audio_buffer_resize(
                            device->buffers[DEVICE_PORT_TYPE_SEND][port], size))
            {
                device->buffer_size = MIN(device->buffer_size, size);
                return false;
            }
        }
        else
        {
            assert(device->buffers[DEVICE_PORT_TYPE_SEND][port] == NULL);
            assert(device->direct_send[port] == NULL);
        }
    }
    device->buffer_size = size;
    return true;
}


void Device_set_node(Device* device, Device_node* node)
{
    assert(device != NULL);
    device->node = node;
    return;
}


bool Device_register_port(Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    if (device->reg[type][port])
    {
        if (type == DEVICE_PORT_TYPE_RECEIVE)
        {
            assert(device->buffers[type][port] != NULL);
        }
        else
        {
            assert(device->buffers[type][port] != NULL ||
                   device->direct_send[port] != NULL);
        }
        return true;
    }
    assert(device->buffers[type][port] == NULL);
    device->buffers[type][port] = new_Audio_buffer(device->buffer_size);
    if (device->buffers[type][port] == NULL)
    {
        return false;
    }
    device->reg[type][port] = true;
    return true;
}


void Device_unregister_port(Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    if (!device->reg[type][port])
    {
        if (type == DEVICE_PORT_TYPE_RECEIVE)
        {
            assert(device->buffers[type][port] == NULL);
        }
        else
        {
            assert(device->buffers[type][port] == NULL &&
                   device->direct_send[port] == NULL);
        }
        return;
    }
    if (type == DEVICE_PORT_TYPE_RECEIVE)
    {
        assert(device->buffers[type][port] != NULL);
        del_Audio_buffer(device->buffers[type][port]);
        device->buffers[type][port] = NULL;
    }
    else if (device->buffers[type][port] != NULL)
    {
        del_Audio_buffer(device->buffers[type][port]);
        device->buffers[type][port] = NULL;
    }
    device->reg[type][port] = false;
    return;
}


Audio_buffer* Device_get_buffer(Device* device,
                                Device_port_type type,
                                int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);
    if (type == DEVICE_PORT_TYPE_RECEIVE)
    {
        assert(device->reg[type][port] ==
               (device->buffers[type][port] != NULL));
        return device->buffers[type][port];
    }
    if (!device->reg[type][port])
    {
        return NULL;
    }
    if (device->direct_send[port] != NULL)
    {
        return device->direct_send[port];
    }
    assert(device->buffers[type][port] != NULL);
    return device->buffers[type][port];
}


void Device_clear_buffers(Device* device)
{
    assert(device != NULL);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (device->reg[DEVICE_PORT_TYPE_RECEIVE][port])
        {
            assert(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port] != NULL);
            Audio_buffer_clear(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port]);
        }
        if (device->reg[DEVICE_PORT_TYPE_SEND][port])
        {
            assert(device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL ||
                    device->direct_send[port] != NULL);
            if (device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL)
            {
                Audio_buffer_clear(device->buffers[DEVICE_PORT_TYPE_SEND][port]);
            }
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
            assert(device->reg[DEVICE_PORT_TYPE_RECEIVE][port]);
            del_Audio_buffer(device->buffers[DEVICE_PORT_TYPE_RECEIVE][port]);
        }
        if (device->buffers[DEVICE_PORT_TYPE_SEND][port] != NULL)
        {
            assert(device->reg[DEVICE_PORT_TYPE_SEND][port]);
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


