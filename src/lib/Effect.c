

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
#include <Device.h>
#include <DSP_table.h>
#include <Effect.h>
#include <xassert.h>
#include <xmemory.h>


struct Effect
{
    Device parent;
    Connections* connections;
    DSP_table* dsps;
};


static void Effect_reset(Device* device);

static bool Effect_set_mix_rate(Device* device, uint32_t mix_rate);

static bool Effect_set_buffer_size(Device* device, uint32_t size);

static bool Effect_sync(Device* device);


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
    Device_register_port(&eff->parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&eff->parent, DEVICE_PORT_TYPE_SEND, 0);
    eff->dsps = new_DSP_table(KQT_DSPS_MAX);
    if (eff->dsps == NULL)
    {
        del_Effect(eff);
        return NULL;
    }
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


void del_Effect(Effect* eff)
{
    if (eff == NULL)
    {
        return;
    }
    del_Connections(eff->connections);
    del_DSP_table(eff->dsps);
    xfree(eff);
    return;
}


