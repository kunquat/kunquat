

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Audio_buffer.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_gc.h>
#include <Envelope.h>
#include <math_common.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct DSP_gc
{
    DSP parent;
    Envelope* map;
} DSP_gc;


static bool DSP_gc_sync(Device* device);
static bool DSP_gc_update_key(Device* device, const char* key);

static void DSP_gc_process(Device* device,
                           uint32_t start,
                           uint32_t until,
                           uint32_t freq,
                           double tempo);

static void del_DSP_gc(DSP* dsp);


DSP* new_DSP_gc(uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    DSP_gc* gc = xalloc(DSP_gc);
    if (gc == NULL)
    {
        return NULL;
    }
    if (!DSP_init(&gc->parent, del_DSP_gc, DSP_gc_process,
                  buffer_size, mix_rate))
    {
        xfree(gc);
        return NULL;
    }
    Device_set_sync(&gc->parent.parent, DSP_gc_sync);
    Device_set_update_key(&gc->parent.parent, DSP_gc_update_key);
    gc->map = NULL;
    gc->map = new_Envelope(4, 0, 1, 0, 0, 1, 0);
    if (gc->map == NULL)
    {
        del_DSP_gc(&gc->parent);
        return NULL;
    }
    Envelope_set_node(gc->map, 0, 0);
    Envelope_set_node(gc->map, 1, 1);
    Envelope_set_first_lock(gc->map, true, false);
    Envelope_set_last_lock(gc->map, true, false);
    Device_register_port(&gc->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&gc->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    //fprintf(stderr, "Created gaincomp %p\n", (void*)gc);
    return &gc->parent;
}


static bool DSP_gc_sync(Device* device)
{
    assert(device != NULL);
    if (!DSP_gc_update_key(device, "p_map.jsone"))
    {
        return false;
    }
    return true;
}


static bool DSP_gc_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);
    DSP_gc* gc = (DSP_gc*)device;
    Device_params* params = gc->parent.conf->params;
    if (string_eq(key, "p_map.jsone"))
    {
        (void)params;
        // TODO: update envelope
    }
    return true;
}


static void DSP_gc_process(Device* device,
                           uint32_t start,
                           uint32_t until,
                           uint32_t freq,
                           double tempo)
{
    assert(device != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    (void)freq;
    (void)tempo;
    DSP_gc* gc = (DSP_gc*)device;
    assert(string_eq(gc->parent.type, "gaincomp"));
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    for (uint32_t i = start; i < until; ++i)
    {
        kqt_frame val_l = fabs(in_data[0][i]);
        kqt_frame val_r = fabs(in_data[1][i]);
        val_l = Envelope_get_value(gc->map, MIN(val_l, 1));
        val_r = Envelope_get_value(gc->map, MIN(val_r, 1));
        if (in_data[0][i] < 0)
        {
            val_l = -val_l;
        }
        if (in_data[1][i] < 0)
        {
            val_r = -val_r;
        }
        out_data[0][i] += val_l;
        out_data[1][i] += val_r;
        assert(!isnan(out_data[0][i]) || isnan(in_data[0][i]));
        assert(!isnan(out_data[0][i]) || isnan(in_data[1][i]));
    }
    return;
}


static void del_DSP_gc(DSP* dsp)
{
    if (dsp == NULL)
    {
        return;
    }
    assert(string_eq(dsp->type, "gaincomp"));
    DSP_gc* gc = (DSP_gc*)dsp;
    del_Envelope(gc->map);
    xfree(gc);
    return;
}


