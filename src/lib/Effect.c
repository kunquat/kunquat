

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Connections.h>
#include <Connections_search.h>
#include <Device.h>
#include <DSP_table.h>
#include <Effect.h>
#include <Effect_interface.h>
#include <xassert.h>
#include <xmemory.h>


struct Effect
{
    Device parent;
    Effect_interface* out_iface;
    Effect_interface* in_iface;
    Connections* connections;
    DSP_table* dsps;
};


static void Effect_reset(Device* device);

static bool Effect_set_mix_rate(Device* device, uint32_t mix_rate);

static bool Effect_set_buffer_size(Device* device, uint32_t size);

static bool Effect_sync(Device* device);

static void Effect_process(Device* device,
                           uint32_t start,
                           uint32_t until,
                           uint32_t freq,
                           double tempo);


Effect* new_Effect(uint32_t buf_len,
                   uint32_t mix_rate)
{
    assert(buf_len > 0);
    assert(mix_rate > 0);
    Effect* eff = xalloc(Effect);
    if (eff == NULL)
    {
        return NULL;
    }
    eff->out_iface = NULL;
    eff->in_iface = NULL;
    eff->connections = NULL;
    eff->dsps = NULL;
    if (!Device_init(&eff->parent, buf_len, mix_rate))
    {
        del_Effect(eff);
        return NULL;
    }
    Device_set_reset(&eff->parent, Effect_reset);
    Device_set_mix_rate_changer(&eff->parent, Effect_set_mix_rate);
    Device_set_buffer_size_changer(&eff->parent, Effect_set_buffer_size);
    Device_set_sync(&eff->parent, Effect_sync);
    Device_set_process(&eff->parent, Effect_process);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_register_port(&eff->parent, DEVICE_PORT_TYPE_RECEIVE, port);
        Device_register_port(&eff->parent, DEVICE_PORT_TYPE_SEND, port);
    }

    eff->out_iface = new_Effect_interface(buf_len, mix_rate);
    eff->in_iface = new_Effect_interface(buf_len, mix_rate);
    eff->dsps = new_DSP_table(KQT_DSPS_MAX);
    if (eff->out_iface == NULL || eff->in_iface == NULL || eff->dsps == NULL)
    {
        del_Effect(eff);
        return NULL;
    }
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_register_port(&eff->out_iface->parent,
                             DEVICE_PORT_TYPE_SEND, port);
        Device_register_port(&eff->in_iface->parent,
                             DEVICE_PORT_TYPE_SEND, port);
    }
    Device_register_port(&eff->out_iface->parent,
                         DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&eff->out_iface->parent,
                         DEVICE_PORT_TYPE_RECEIVE, 1);
    return eff;
}


bool Effect_parse_header(Effect* eff, char* str, Read_state* state)
{
    assert(eff != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    (void)str;
    assert(false);
    return false;
}


DSP* Effect_get_dsp(Effect* eff, int index)
{
    assert(eff != NULL);
    assert(index >= 0);
    assert(index < KQT_DSPS_MAX);
    return DSP_table_get_dsp(eff->dsps, index);
}


DSP_table* Effect_get_dsps(Effect* eff)
{
    assert(eff != NULL);
    return eff->dsps;
}


void Effect_set_connections(Effect* eff, Connections* graph)
{
    assert(eff != NULL);
    if (eff->connections != NULL)
    {
        del_Connections(eff->connections);
    }
    eff->connections = graph;
    return;
}


bool Effect_prepare_connections(Effect* eff)
{
    assert(eff != NULL);
    if (eff->connections == NULL)
    {
        return true;
    }
    Device_remove_direct_buffers(&eff->out_iface->parent);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; port += 2)
    {
        Device_set_direct_send(&eff->out_iface->parent, port,
                               Device_get_buffer(&eff->parent,
                                                 DEVICE_PORT_TYPE_SEND,
                                                 port / 2));
        Device_set_direct_receive(&eff->out_iface->parent, port);
    }
    Device_remove_direct_buffers(&eff->in_iface->parent);
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_set_direct_send(&eff->in_iface->parent, port,
                               Device_get_buffer(&eff->parent,
                                                 DEVICE_PORT_TYPE_RECEIVE,
                                                 port));
    }
    return Connections_prepare(eff->connections);
}


Device* Effect_get_input_interface(Effect* eff)
{
    assert(eff != NULL);
    return &eff->in_iface->parent;
}


Device* Effect_get_output_interface(Effect* eff)
{
    assert(eff != NULL);
    return &eff->out_iface->parent;
}


static void Effect_reset(Device* device)
{
    assert(device != NULL);
    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL)
        {
            Device_reset((Device*)dsp);
        }
    }
    return;
}


static bool Effect_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL && !Device_set_mix_rate((Device*)dsp, mix_rate))
        {
            return false;
        }
    }
    return true;
}


static bool Effect_set_buffer_size(Device* device, uint32_t size)
{
    assert(device != NULL);
    assert(size > 0);
    assert(size <= KQT_BUFFER_SIZE_MAX);
    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL && !Device_set_buffer_size((Device*)dsp, size))
        {
            return false;
        }
    }
    return true;
}


static bool Effect_sync(Device* device)
{
    assert(device != NULL);
    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL && !Device_sync((Device*)dsp))
        {
            return false;
        }
    }
    return true;
}


static void Effect_process(Device* device,
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
    Effect* eff = (Effect*)device;
    Connections_clear_buffers(eff->connections, start, until);
    Connections_mix(eff->connections, start, until, freq, tempo);
    return;
}


void del_Effect(Effect* eff)
{
    if (eff == NULL)
    {
        return;
    }
    del_Effect_interface(eff->out_iface);
    del_Effect_interface(eff->in_iface);
    del_Connections(eff->connections);
    del_DSP_table(eff->dsps);
    Device_uninit(&eff->parent);
    xfree(eff);
    return;
}


