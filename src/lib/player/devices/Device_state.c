

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Device_state.h>

#include <containers/Bit_array.h>
#include <debug/assert.h>
#include <init/devices/Device.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffer.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


bool Device_state_init(
        Device_state* ds,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(ds != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    ds->device = device;
    ds->device_id = Device_get_id(ds->device);

    ds->node_state = DEVICE_NODE_STATE_NEW;

    ds->audio_rate = audio_rate;
    ds->audio_buffer_size = audio_buffer_size;

    ds->is_stream_state = false;

    ds->in_connected = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            ds->buffers[type][port] = NULL;
    }

    ds->add_buffer = NULL;
    ds->set_audio_rate = NULL;
    ds->set_audio_buffer_size = NULL;
    ds->set_tempo = NULL;
    ds->reset = NULL;
    ds->render_mixed = NULL;
    ds->destroy = NULL;

    ds->in_connected = new_Bit_array(KQT_DEVICE_PORTS_MAX);
    if (ds->in_connected == NULL)
        return false;

    return true;
}


Device_state* new_Device_state_plain(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Device_state* ds = memory_alloc_item(Device_state);
    if (ds == NULL)
        return NULL;

    if (!Device_state_init(ds, device, audio_rate, audio_buffer_size))
    {
        del_Device_state(ds);
        return NULL;
    }

    return ds;
}


int Device_state_cmp(const Device_state* ds1, const Device_state* ds2)
{
    assert(ds1 != NULL);
    assert(ds2 != NULL);

    if (ds1->device_id < ds2->device_id)
        return -1;
    else if (ds1->device_id > ds2->device_id)
        return 1;
    return 0;
}


const Device* Device_state_get_device(const Device_state* ds)
{
    assert(ds != NULL);
    return ds->device;
}


void Device_state_set_node_state(Device_state* ds, Device_node_state node_state)
{
    assert(ds != NULL);
    assert(node_state < DEVICE_NODE_STATE_COUNT);

    ds->node_state = node_state;

    return;
}


Device_node_state Device_state_get_node_state(const Device_state* ds)
{
    assert(ds != NULL);
    return ds->node_state;
}


bool Device_state_set_audio_rate(Device_state* ds, int32_t audio_rate)
{
    assert(ds != NULL);
    assert(audio_rate > 0);

    if (ds->audio_rate == audio_rate)
        return true;

    ds->audio_rate = audio_rate;

    if (ds->set_audio_rate != NULL)
        return ds->set_audio_rate(ds, audio_rate);

    return true;
}


int32_t Device_state_get_audio_rate(const Device_state* ds)
{
    assert(ds != NULL);
    return ds->audio_rate;
}


bool Device_state_set_audio_buffer_size(Device_state* ds, int32_t size)
{
    assert(ds != NULL);
    assert(size >= 0);

    ds->audio_buffer_size = min(ds->audio_buffer_size, size);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            Work_buffer* buffer = ds->buffers[type][port];
            if ((buffer != NULL) && !Work_buffer_resize(buffer, size))
                return false;
        }
    }

    if ((ds->set_audio_buffer_size != NULL) && !ds->set_audio_buffer_size(ds, size))
        return false;

    ds->audio_buffer_size = size;
    return true;
}


bool Device_state_allocate_space(Device_state* ds, char* key)
{
    assert(ds != NULL);
    assert(key != NULL);

    return true;
}


bool Device_state_add_audio_buffer(Device_state* ds, Device_port_type type, int port)
{
    assert(ds != NULL);
    assert(type == DEVICE_PORT_TYPE_RECEIVE || type == DEVICE_PORT_TYPE_SEND);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    if (ds->buffers[type][port] != NULL)
        return true;

    ds->buffers[type][port] = new_Work_buffer(ds->audio_buffer_size);
    if (ds->buffers[type][port] == NULL)
        return false;

    if ((ds->add_buffer != NULL) && !ds->add_buffer(ds, type, port))
        return false;

    return true;
}


void Device_state_clear_audio_buffers(Device_state* ds, int32_t start, int32_t stop)
{
    assert(ds != NULL);
    assert(start >= 0);
    assert(stop >= start);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            Work_buffer* buffer = ds->buffers[type][port];
            if (buffer != NULL)
                Work_buffer_clear(buffer, start, stop);
        }
    }

    Bit_array_clear(ds->in_connected);

    return;
}


Work_buffer* Device_state_get_audio_buffer(
        const Device_state* ds, Device_port_type type, int port)
{
    assert(ds != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    if ((type == DEVICE_PORT_TYPE_RECEIVE) &&
            !Device_state_is_input_port_connected(ds, port))
        return NULL;

    return ds->buffers[type][port];
}


float* Device_state_get_audio_buffer_contents_mut(
        const Device_state* ds, Device_port_type type, int port)
{
    assert(ds != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    Work_buffer* wb = Device_state_get_audio_buffer(ds, type, port);
    if (wb == NULL)
        return NULL;

    return Work_buffer_get_contents_mut(wb);
}


void Device_state_mark_input_port_connected(Device_state* ds, int port)
{
    assert(ds != NULL);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    Bit_array_set(ds->in_connected, port, true);

    return;
}


bool Device_state_is_input_port_connected(const Device_state* ds, int port)
{
    assert(ds != NULL);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    return Bit_array_get(ds->in_connected, port);
}


void Device_state_set_tempo(Device_state* ds, double tempo)
{
    assert(ds != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (ds->set_tempo != NULL)
        ds->set_tempo(ds, tempo);

    return;
}


void Device_state_reset(Device_state* ds)
{
    assert(ds != NULL);

    if (ds->reset != NULL)
        ds->reset(ds);

    return;
}


void Device_state_render_mixed(
        Device_state* ds,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(ds != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    assert(ds->node_state == DEVICE_NODE_STATE_REACHED);

    if (Device_get_mixed_signals(ds->device) && (ds->render_mixed != NULL))
        ds->render_mixed(ds, wbs, buf_start, buf_stop, tempo);

    return;
}


void del_Device_state(Device_state* ds)
{
    if (ds == NULL)
        return;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            del_Work_buffer(ds->buffers[type][port]);
            ds->buffers[type][port] = NULL;
        }
    }

    del_Bit_array(ds->in_connected);
    ds->in_connected = NULL;

    if (ds->destroy != NULL)
        ds->destroy(ds);
    else
        memory_free(ds);

    return;
}


