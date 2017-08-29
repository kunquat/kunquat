

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
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
    ts->has_mixed_audio = false;
    ts->in_connected = NULL;

    for (Device_buffer_type buf_type = DEVICE_BUFFER_MIXED;
            buf_type < DEVICE_BUFFER_TYPES; ++buf_type)
    {
        for (Device_port_type port_type = DEVICE_PORT_TYPE_RECV;
                port_type < DEVICE_PORT_TYPES; ++port_type)
            ts->buffers[buf_type][port_type] = NULL;
    }

    ts->in_connected = new_Bit_array(KQT_DEVICE_PORTS_MAX);
    if (ts->in_connected == NULL)
    {
        del_Device_thread_state(ts);
        return NULL;
    }

    for (Device_buffer_type buf_type = DEVICE_BUFFER_MIXED;
            buf_type < DEVICE_BUFFER_TYPES; ++buf_type)
    {
        for (Device_port_type port_type = DEVICE_PORT_TYPE_RECV;
                port_type < DEVICE_PORT_TYPES; ++port_type)
        {
            ts->buffers[buf_type][port_type] =
                new_Etable(KQT_DEVICE_PORTS_MAX, (void (*)(void*))del_Work_buffer);
            if (ts->buffers[buf_type][port_type] == NULL)
            {
                del_Device_thread_state(ts);
                return NULL;
            }
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

    for (Device_buffer_type buf_type = DEVICE_BUFFER_MIXED;
            buf_type < DEVICE_BUFFER_TYPES; ++buf_type)
    {
        for (Device_port_type port_type = DEVICE_PORT_TYPE_RECV;
                port_type < DEVICE_PORT_TYPES; ++port_type)
        {
            Etable* bufs = ts->buffers[buf_type][port_type];
            const int cap = Etable_get_capacity(bufs);
            for (int port = 0; port < cap; ++port)
            {
                Work_buffer* buffer = Etable_get(bufs, port);
                if ((buffer != NULL) && !Work_buffer_resize(buffer, size))
                    return false;
            }
        }
    }

    return true;
}


static bool Device_thread_state_add_buffer(
        Device_thread_state* ts,
        Device_buffer_type buf_type,
        Device_port_type port_type,
        int port)
{
    rassert(ts != NULL);
    rassert(buf_type < DEVICE_BUFFER_TYPES);
    rassert(port_type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    if (Etable_get(ts->buffers[buf_type][port_type], port) != NULL)
        return true;

    Work_buffer* wb = new_Work_buffer(ts->audio_buffer_size);
    if ((wb == NULL) || !Etable_set(ts->buffers[buf_type][port_type], port, wb))
    {
        del_Work_buffer(wb);
        return false;
    }

    return true;
}


static void Device_thread_state_clear_buffers(
        Device_thread_state* ts,
        Device_buffer_type buf_type,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(ts != NULL);
    rassert(buf_type < DEVICE_BUFFER_TYPES);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECV;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        Etable* bufs = ts->buffers[buf_type][port_type];
        const int cap = Etable_get_capacity(bufs);
        for (int port = 0; port < cap; ++port)
        {
            Work_buffer* buffer = Etable_get(bufs, port);
            if (buffer != NULL)
                Work_buffer_clear(buffer, buf_start, buf_stop);
        }
    }

    Bit_array_clear(ts->in_connected);

    return;
}


static Work_buffer* Device_thread_state_get_buffer(
        const Device_thread_state* ts,
        Device_buffer_type buf_type,
        Device_port_type port_type,
        int port)
{
    rassert(ts != NULL);
    rassert(buf_type < DEVICE_BUFFER_TYPES);
    rassert(port_type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    if ((port_type == DEVICE_PORT_TYPE_RECV) &&
            !Device_thread_state_is_input_port_connected(ts, port))
        return NULL;

    return Etable_get(ts->buffers[buf_type][port_type], port);
}


bool Device_thread_state_add_mixed_buffer(
        Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Device_thread_state_add_buffer(ts, DEVICE_BUFFER_MIXED, type, port);
}


void Device_thread_state_clear_mixed_buffers(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop)
{
    rassert(ts != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    Device_thread_state_clear_buffers(ts, DEVICE_BUFFER_MIXED, buf_start, buf_stop);

    ts->has_mixed_audio = false;

    return;
}


Work_buffer* Device_thread_state_get_mixed_buffer(
        const Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Etable_get(ts->buffers[DEVICE_BUFFER_MIXED][type], port);
}


Work_buffer* Device_thread_state_get_connected_mixed_buffer(
        const Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Device_thread_state_get_buffer(ts, DEVICE_BUFFER_MIXED, type, port);
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


bool Device_thread_state_add_voice_buffer(
        Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Device_thread_state_add_buffer(ts, DEVICE_BUFFER_VOICE, type, port);
}


void Device_thread_state_clear_voice_buffers(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop)
{
    rassert(ts != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    Device_thread_state_clear_buffers(ts, DEVICE_BUFFER_VOICE, buf_start, buf_stop);

    return;
}


Work_buffer* Device_thread_state_get_voice_buffer(
        const Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Device_thread_state_get_buffer(ts, DEVICE_BUFFER_VOICE, type, port);
}


float* Device_thread_state_get_voice_buffer_contents(
        const Device_thread_state* ts, Device_port_type type, int port)
{
    rassert(ts != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    Work_buffer* wb = Device_thread_state_get_voice_buffer(ts, type, port);
    if (wb == NULL)
        return NULL;

    return Work_buffer_get_contents_mut(wb);
}


void Device_thread_state_mark_mixed_audio(Device_thread_state* ts)
{
    rassert(ts != NULL);

    ts->has_mixed_audio = true;

    return;
}


void Device_thread_state_mix_voice_signals(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop)
{
    rassert(ts != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    for (int32_t port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Work_buffer* mixed_buffer =
            Device_thread_state_get_mixed_buffer(ts, DEVICE_PORT_TYPE_SEND, port);
        if (mixed_buffer == NULL)
            continue;

        const Work_buffer* voice_buffer =
            Device_thread_state_get_voice_buffer(ts, DEVICE_PORT_TYPE_SEND, port);
        rassert(voice_buffer != NULL);

        Work_buffer_mix(mixed_buffer, voice_buffer, buf_start, buf_stop);
        Device_thread_state_mark_mixed_audio(ts);
    }

    return;
}


bool Device_thread_state_has_mixed_audio(const Device_thread_state* ts)
{
    rassert(ts != NULL);
    return ts->has_mixed_audio;
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

    for (Device_buffer_type buf_type = DEVICE_BUFFER_MIXED;
            buf_type < DEVICE_BUFFER_TYPES; ++buf_type)
    {
        for (Device_port_type port_type = DEVICE_PORT_TYPE_RECV;
                port_type < DEVICE_PORT_TYPES; ++port_type)
            del_Etable(ts->buffers[buf_type][port_type]);
    }

    memory_free(ts);

    return;
}


