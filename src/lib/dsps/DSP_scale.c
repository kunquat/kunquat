

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <DSP.h>
#include <DSP_scale.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct DSP_scale
{
    DSP parent;
} DSP_scale;


static void DSP_scale_process(Device* device, uint32_t start, uint32_t until);


static void del_DSP_scale(DSP* dsp);


DSP* new_DSP_scale(uint32_t buffer_size)
{
    DSP_scale* scale = xalloc(DSP_scale);
    if (scale == NULL)
    {
        return NULL;
    }
    if (!DSP_init(&scale->parent, del_DSP_scale,
                  DSP_scale_process, buffer_size))
    {
        xfree(scale);
        return NULL;
    }
    Device_register_port(&scale->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&scale->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    return &scale->parent;
}


static void DSP_scale_process(Device* device, uint32_t start, uint32_t until)
{
    assert(device != NULL);
    DSP_scale* scale = (DSP_scale*)device;
    assert(strcmp(scale->parent.type, "scale") == 0);
    assert(scale->parent.conf != NULL);
    assert(scale->parent.conf->params != NULL);
    Audio_buffer* in = Device_get_buffer(device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Audio_buffer* out = Device_get_buffer(device, DEVICE_PORT_TYPE_SEND, 0);
    if (in == NULL || out == NULL)
    {
        return;
    }
    assert(in != out);
    kqt_frame* in_data[] =
    {
        Audio_buffer_get_buffer(in, 0),
        Audio_buffer_get_buffer(in, 1),
    };
    kqt_frame* out_data[] =
    {
        Audio_buffer_get_buffer(out, 0),
        Audio_buffer_get_buffer(out, 1),
    };
    double factor = 1;
    double* dB_arg = Device_params_get_float(scale->parent.conf->params,
                                             "p_volume.jsonf");
    if (dB_arg != NULL && isfinite(*dB_arg))
    {
        factor = exp2(*dB_arg / 6);
    }
    for (uint32_t frame = start; frame < until; ++frame)
    {
        out_data[0][frame] += in_data[0][frame] * factor;
        out_data[1][frame] += in_data[1][frame] * factor;
    }
    return;
}


static void del_DSP_scale(DSP* dsp)
{
    assert(dsp != NULL);
    assert(strcmp(dsp->type, "scale") == 0);
    DSP_scale* scale = (DSP_scale*)dsp;
    Device_uninit(&scale->parent.parent);
    xfree(scale);
    return;
}


