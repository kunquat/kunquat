

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/DSP.h>
#include <devices/dsps/DSP_common.h>
#include <devices/dsps/DSP_gc.h>
#include <Envelope.h>
#include <math_common.h>
#include <memory.h>
#include <string_common.h>


typedef struct DSP_gc
{
    Device_impl parent;

    const Envelope* map;
} DSP_gc;


static bool DSP_gc_set_map(
        Device_impl* dimpl,
        Device_key_indices indices,
        const Envelope* value);

static void DSP_gc_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_DSP_gc(Device_impl* dsp_impl);


Device_impl* new_DSP_gc(DSP* dsp)
{
    DSP_gc* gc = memory_alloc_item(DSP_gc);
    if (gc == NULL)
        return NULL;

    if (!Device_impl_init(&gc->parent, del_DSP_gc))
    {
        memory_free(gc);
        return NULL;
    }

    gc->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_gc_process);

    gc->map = NULL;

    if (!Device_impl_register_set_envelope(
                &gc->parent, "p_e_map.json", NULL, DSP_gc_set_map, NULL))
    {
        del_DSP_gc(&gc->parent);
        return NULL;
    }

    Device_register_port(gc->parent.device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(gc->parent.device, DEVICE_PORT_TYPE_SEND, 0);

    return &gc->parent;
}


static bool DSP_gc_set_map(
        Device_impl* dimpl,
        Device_key_indices indices,
        const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_gc* gc = (DSP_gc*)dimpl;

    bool valid = true;
    if (value != NULL && Envelope_node_count(value) > 1)
    {
        double* node = Envelope_get_node(value, 0);
        if (node[0] != 0)
            valid = false;

        node = Envelope_get_node(value, Envelope_node_count(value) - 1);
        if (node[0] != 1)
            valid = false;

        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            node = Envelope_get_node(value, i);
            if (node[1] < 0)
            {
                valid = false;
                break;
            }
        }
    }
    else
    {
        valid = false;
    }

    gc->map = valid ? value : NULL;

    return true;
}


static void DSP_gc_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    Device_state* ds = Device_states_get_state(states, Device_get_id(device));
    assert(ds != NULL);

    (void)freq;
    (void)tempo;
    DSP_gc* gc = (DSP_gc*)device->dimpl;
    //assert(string_eq(gc->parent.type, "gaincomp"));
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(ds, 0, in_data);
    DSP_get_raw_output(ds, 0, out_data);

    if (gc->map != NULL)
    {
        for (uint32_t i = start; i < until; ++i)
        {
            kqt_frame val_l = fabs(in_data[0][i]);
            kqt_frame val_r = fabs(in_data[1][i]);
            val_l = Envelope_get_value(gc->map, MIN(val_l, 1));
            val_r = Envelope_get_value(gc->map, MIN(val_r, 1));
            if (in_data[0][i] < 0)
                val_l = -val_l;
            if (in_data[1][i] < 0)
                val_r = -val_r;

            out_data[0][i] += val_l;
            out_data[1][i] += val_r;
            assert(!isnan(out_data[0][i]) || isnan(in_data[0][i]));
            assert(!isnan(out_data[0][i]) || isnan(in_data[1][i]));
        }
    }
    else
    {
        for (uint32_t i = start; i < until; ++i)
        {
            out_data[0][i] += in_data[0][i];
            out_data[1][i] += in_data[1][i];
        }
    }

    return;
}


static void del_DSP_gc(Device_impl* dsp_impl)
{
    if (dsp_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "gaincomp"));
    DSP_gc* gc = (DSP_gc*)dsp_impl;
    memory_free(gc);

    return;
}


