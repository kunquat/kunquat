

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Device_thread_state.h>

#include <containers/Bit_array.h>
#include <containers/Etable.h>
#include <debug/assert.h>
#include <memory.h>
#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


Device_thread_state* new_Device_thread_state(
        uint32_t device_id, int32_t audio_buffer_size)
{
    rassert(audio_buffer_size >= 0);

    Device_thread_state* ts = memory_alloc_item(Device_thread_state);
    if (ts == NULL)
        return NULL;

    ts->device_id = device_id;
    ts->audio_buffer_size = audio_buffer_size;
    ts->node_state = DEVICE_NODE_STATE_NEW;
    ts->in_connected = NULL;

    for (Device_port_type type = DEVICE_PORT_TYPE_RECV; type < DEVICE_PORT_TYPES; ++type)
        ts->buffers[type] = NULL;

    ts->in_connected = new_Bit_array(KQT_DEVICE_PORTS_MAX);
    if (ts->in_connected == NULL)
    {
        del_Device_thread_state(ts);
        return NULL;
    }

    for (Device_port_type type = DEVICE_PORT_TYPE_RECV; type < DEVICE_PORT_TYPES; ++type)
    {
        ts->buffers[type] =
            new_Etable(KQT_DEVICE_PORTS_MAX, (void (*)(void*))del_Work_buffer);
        if (ts->buffers[type] == NULL)
        {
            del_Device_thread_state(ts);
            return NULL;
        }
    }

    return ts;
}


int Device_thread_state_cmp(
        const Device_thread_state* ts1, const Device_thread_state* ts2)
{
    rassert(ts1 != NULL);
    rassert(ts2 != NULL);

    if (ts1->device_id < ts2->device_id)
        return -1;
    else if (ts1->device_id > ts2->device_id)
        return 1;
    return 0;
}


void Device_thread_state_set_node_state(
        Device_thread_state* ts, Device_node_state node_state)
{
    rassert(ts != NULL);
    rassert(node_state < DEVICE_NODE_STATE_COUNT);

    ts->node_state = node_state;

    return;
}


Device_node_state Device_thread_state_get_node_state(const Device_thread_state* ts)
{
    rassert(ts != NULL);
    return ts->node_state;
}


bool Device_thread_state_set_audio_buffer_size(Device_thread_state* ts, int size)
{
    rassert(ts != NULL);
    rassert(size >= 0);

    for (Device_port_type type = DEVICE_PORT_TYPE_RECV; type < DEVICE_PORT_TYPES; ++type)
    {
        for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
        {
            Work_buffer* buffer = Etable_get(ts->buffers[type], port);
            if ((buffer != NULL) && !Work_buffer_resize(buffer, size))
                return false;
        }
    }

    return true;
}


bool Device_thread_state_add_mixed_buffer(
        Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type == DEVICE_PORT_TYPE_RECV || type == DEVICE_PORT_TYPE_SEND);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    if (Etable_get(ts->buffers[type], port) != NULL)
        return true;

    Work_buffer* wb = new_Work_buffer(ts->audio_buffer_size);
    if ((wb == NULL) || !Etable_set(ts->buffers[type], port, wb))
    {
        del_Work_buffer(wb);
        return false;
    }

    return true;
}


void Device_thread_state_clear_mixed_buffers(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop)
{
    rassert(ts != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECV;
                type < DEVICE_PORT_TYPES; ++type)
        {
            Work_buffer* buffer = Etable_get(ts->buffers[type], port);
            if (buffer != NULL)
                Work_buffer_clear(buffer, buf_start, buf_stop);
        }
    }

    Bit_array_clear(ts->in_connected);

    return;
}


Work_buffer* Device_thread_state_get_mixed_buffer(
        const Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    if ((type == DEVICE_PORT_TYPE_RECV) &&
            !Device_thread_state_is_input_port_connected(ts, port))
        return NULL;

    return Etable_get(ts->buffers[type], port);
}


float* Device_thread_state_get_mixed_buffer_contents_mut(
        const Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    Work_buffer* wb = Device_thread_state_get_mixed_buffer(ts, type, port);
    if (wb == NULL)
        return NULL;

    return Work_buffer_get_contents_mut(wb);
}


void Device_thread_state_mark_input_port_connected(Device_thread_state* ts, int port)
{
    rassert(ts != NULL);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    Bit_array_set(ts->in_connected, port, true);

    return;
}


bool Device_thread_state_is_input_port_connected(
        const Device_thread_state* ts, int port)
{
    rassert(ts != NULL);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Bit_array_get(ts->in_connected, port);
}


void del_Device_thread_state(Device_thread_state* ts)
{
    if (ts == NULL)
        return;

    del_Bit_array(ts->in_connected);

    for (Device_port_type type = DEVICE_PORT_TYPE_RECV; type < DEVICE_PORT_TYPES; ++type)
        del_Etable(ts->buffers[type]);

    memory_free(ts);

    return;
}


