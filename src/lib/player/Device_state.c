

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/Device.h>
#include <math_common.h>
#include <memory.h>
#include <player/Device_state.h>
#include <xassert.h>


void Device_state_init(
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
    ds->audio_rate = audio_rate;
    ds->audio_buffer_size = audio_buffer_size;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            ds->buffers[type][port] = NULL;
    }

    ds->destroy = NULL;

    return;
}


Device_state* new_Device_state_plain(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Device_state* ds = memory_alloc_item(Device_state);
    if (ds == NULL)
        return NULL;

    Device_state_init(ds, device, audio_rate, audio_buffer_size);

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


bool Device_state_set_audio_rate(Device_state* ds, int32_t rate)
{
    assert(ds != NULL);
    assert(rate > 0);

    if (ds->audio_rate == rate)
        return true;

    ds->audio_rate = rate;

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

    ds->audio_buffer_size = MIN(ds->audio_buffer_size, size);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            if (!Audio_buffer_resize(ds->buffers[type][port], size))
                return false;
        }
    }

    ds->audio_buffer_size = size;
    return true;
}


bool Device_state_allocate_space(Device_state* ds, char* key)
{
    assert(ds != NULL);
    assert(key != NULL);

    return true;
}


bool Device_state_add_audio_buffer(
        Device_state* ds,
        Device_port_type type,
        int port)
{
    assert(ds != NULL);
    assert(type == DEVICE_PORT_TYPE_RECEIVE || type == DEVICE_PORT_TYPE_SEND);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    if (ds->buffers[type][port] != NULL)
        return true;

    ds->buffers[type][port] = new_Audio_buffer(ds->audio_buffer_size);
    if (ds->buffers[type][port] == NULL)
        return false;

    return true;
}


void Device_state_clear_audio_buffers(
        Device_state* ds,
        uint32_t start,
        uint32_t stop)
{
    assert(ds != NULL);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
        {
            Audio_buffer* buffer = ds->buffers[type][port];
            if (buffer != NULL)
                Audio_buffer_clear(buffer, start, stop);
        }
    }
}


Audio_buffer* Device_state_get_audio_buffer(
        const Device_state* ds,
        Device_port_type type,
        int port)
{
    assert(ds != NULL);
    assert(type == DEVICE_PORT_TYPE_RECEIVE || type == DEVICE_PORT_TYPE_SEND);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    return ds->buffers[type][port];
}


void Device_state_reset(Device_state* ds)
{
    assert(ds != NULL);

    return;
}


void del_Device_state(Device_state* ds)
{
    if (ds == NULL)
        return;

    if (ds->destroy != NULL)
        ds->destroy(ds);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            del_Audio_buffer(ds->buffers[type][port]);
    }
    memory_free(ds);

    return;
}


