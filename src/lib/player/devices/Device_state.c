

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
    rassert(ds != NULL);
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    ds->device = device;
    ds->device_id = Device_get_id(ds->device);

    /*
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        ds->thread_states[i] = NULL;
    // */

    ds->audio_rate = audio_rate;
    ds->audio_buffer_size = audio_buffer_size;

    ds->is_stream_state = false;

    ds->add_buffer = NULL;
    ds->set_audio_rate = NULL;
    ds->set_audio_buffer_size = NULL;
    ds->set_tempo = NULL;
    ds->reset = NULL;
    ds->render_mixed = NULL;
    ds->destroy = NULL;

    /*
    ds->thread_states[0] = new_Device_thread_state();
    if (ds->thread_states[0] == NULL)
        return false;
    // */

    return true;
}


Device_state* new_Device_state_plain(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

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
    rassert(ds1 != NULL);
    rassert(ds2 != NULL);

    if (ds1->device_id < ds2->device_id)
        return -1;
    else if (ds1->device_id > ds2->device_id)
        return 1;
    return 0;
}


const Device* Device_state_get_device(const Device_state* ds)
{
    rassert(ds != NULL);
    return ds->device;
}


/*
void Device_state_set_node_state(
        Device_state* ds, int thread_id, Device_node_state node_state)
{
    rassert(ds != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(ds->thread_states[thread_id] != NULL);
    rassert(node_state < DEVICE_NODE_STATE_COUNT);

    ds->thread_states[thread_id]->node_state = node_state;

    return;
}
// */


/*
Device_node_state Device_state_get_node_state(const Device_state* ds, int thread_id)
{
    rassert(ds != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(ds->thread_states[thread_id] != NULL);

    return ds->thread_states[thread_id]->node_state;
}
// */


bool Device_state_set_audio_rate(Device_state* ds, int32_t audio_rate)
{
    rassert(ds != NULL);
    rassert(audio_rate > 0);

    if (ds->audio_rate == audio_rate)
        return true;

    ds->audio_rate = audio_rate;

    if (ds->set_audio_rate != NULL)
        return ds->set_audio_rate(ds, audio_rate);

    return true;
}


int32_t Device_state_get_audio_rate(const Device_state* ds)
{
    rassert(ds != NULL);
    return ds->audio_rate;
}


bool Device_state_set_audio_buffer_size(Device_state* ds, int32_t size)
{
    rassert(ds != NULL);
    rassert(size >= 0);

    ds->audio_buffer_size = min(ds->audio_buffer_size, size);

    /*
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
    {
        if ((ds->thread_states[i] != NULL) &&
                !Device_thread_state_set_audio_buffer_size(ds->thread_states[i], size))
            return false;
    }
    // */

    if ((ds->set_audio_buffer_size != NULL) && !ds->set_audio_buffer_size(ds, size))
        return false;

    ds->audio_buffer_size = size;
    return true;
}


/*
bool Device_state_allocate_space(Device_state* ds, char* key)
{
    rassert(ds != NULL);
    rassert(key != NULL);

    return true;
}
// */


bool Device_state_add_audio_buffer(Device_state* ds, Device_port_type type, int port)
{
    rassert(ds != NULL);
    rassert(type == DEVICE_PORT_TYPE_RECEIVE || type == DEVICE_PORT_TYPE_SEND);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    /*
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
    {
        if ((ds->thread_states[i] != NULL) && !Device_thread_state_add_audio_buffer(
                    ds->thread_states[i], type, port, ds->audio_buffer_size))
            return false;
    }
    // */

    if ((ds->add_buffer != NULL) && !ds->add_buffer(ds, type, port))
        return false;

    return true;
}


/*
void Device_state_clear_audio_buffers(
        Device_state* ds, int32_t buf_start, int32_t buf_stop)
{
    rassert(ds != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    for (int i = 0; i < KQT_THREADS_MAX; ++i)
    {
        Device_thread_state* ts = ds->thread_states[i];
        if (ts != NULL)
            Device_thread_state_clear_audio_buffers(ts, buf_start, buf_stop);
    }

    return;
}
// */


/*
Work_buffer* Device_state_get_audio_buffer(
        const Device_state* ds, int thread_id, Device_port_type type, int port)
{
    rassert(ds != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(ds->thread_states[thread_id] != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Device_thread_state_get_audio_buffer(
            ds->thread_states[thread_id], type, port);
}
// */


/*
float* Device_state_get_audio_buffer_contents_mut(
        const Device_state* ds, int thread_id, Device_port_type type, int port)
{
    rassert(ds != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(ds->thread_states[thread_id] != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    Work_buffer* wb = Device_state_get_audio_buffer(ds, thread_id, type, port);
    if (wb == NULL)
        return NULL;

    return Work_buffer_get_contents_mut(wb);
}
// */


/*
void Device_state_mark_input_port_connected(Device_state* ds, int thread_id, int port)
{
    rassert(ds != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(ds->thread_states[thread_id] != NULL);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    Device_thread_state_mark_input_port_connected(ds->thread_states[thread_id], port);

    return;
}
// */


/*
bool Device_state_is_input_port_connected(
        const Device_state* ds, int thread_id, int port)
{
    rassert(ds != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(ds->thread_states[thread_id] != NULL);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return Device_thread_state_is_input_port_connected(
            ds->thread_states[thread_id], port);
}
// */


void Device_state_set_tempo(Device_state* ds, double tempo)
{
    rassert(ds != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    if (ds->set_tempo != NULL)
        ds->set_tempo(ds, tempo);

    return;
}


void Device_state_reset(Device_state* ds)
{
    rassert(ds != NULL);

    if (ds->reset != NULL)
        ds->reset(ds);

    return;
}


void Device_state_render_mixed(
        Device_state* ds,
        Device_thread_state* ts,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(ds != NULL);
    rassert(ts != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    //rassert(ds->thread_states[0]->node_state == DEVICE_NODE_STATE_REACHED);

    if (Device_get_mixed_signals(ds->device) && (ds->render_mixed != NULL))
        ds->render_mixed(ds, ts, wbs, buf_start, buf_stop, tempo);

    return;
}


void del_Device_state(Device_state* ds)
{
    if (ds == NULL)
        return;

    /*
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
    {
        del_Device_thread_state(ds->thread_states[i]);
        ds->thread_states[i] = NULL;
    }
    // */

    if (ds->destroy != NULL)
        ds->destroy(ds);
    else
        memory_free(ds);

    return;
}


