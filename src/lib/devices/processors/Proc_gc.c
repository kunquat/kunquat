

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
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
#include <devices/param_types/Envelope.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_gc.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>


typedef struct Proc_gc
{
    Device_impl parent;

    const Envelope* map;
} Proc_gc;


static Set_envelope_func Proc_gc_set_map;

static bool Proc_gc_init(Device_impl* dimpl);

static void Proc_gc_process(
        const Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_Proc_gc(Device_impl* dimpl);


Device_impl* new_Proc_gc(Processor* proc)
{
    Proc_gc* gc = memory_alloc_item(Proc_gc);
    if (gc == NULL)
        return NULL;

    gc->parent.device = (Device*)proc;

    Device_impl_register_init(&gc->parent, Proc_gc_init);
    Device_impl_register_destroy(&gc->parent, del_Proc_gc);

    return &gc->parent;
}


static bool Proc_gc_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_gc* gc = (Proc_gc*)dimpl;

    Device_set_process(gc->parent.device, Proc_gc_process);

    gc->map = NULL;

    if (!Device_impl_register_set_envelope(
                &gc->parent, "p_e_map.json", NULL, Proc_gc_set_map, NULL))
        return false;

    return true;
}


static bool Proc_gc_set_map(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    Proc_gc* gc = (Proc_gc*)dimpl;

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


static void Proc_gc_process(
        const Device* device,
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
    Proc_gc* gc = (Proc_gc*)device->dimpl;
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    get_raw_input(ds, 0, in_data);
    get_raw_output(ds, 0, out_data);

    if (gc->map != NULL)
    {
        for (uint32_t i = start; i < until; ++i)
        {
            kqt_frame val_l = fabs(in_data[0][i]);
            kqt_frame val_r = fabs(in_data[1][i]);
            val_l = Envelope_get_value(gc->map, min(val_l, 1));
            val_r = Envelope_get_value(gc->map, min(val_r, 1));
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


static void del_Proc_gc(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_gc* gc = (Proc_gc*)dimpl;
    memory_free(gc);

    return;
}


