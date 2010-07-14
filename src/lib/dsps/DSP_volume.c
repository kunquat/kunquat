

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
#include <DSP_common.h>
#include <DSP_volume.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct DSP_volume
{
    DSP parent;
} DSP_volume;


static void DSP_volume_process(Device* device,
                               uint32_t start,
                               uint32_t until,
                               uint32_t freq,
                               double tempo);


static void del_DSP_volume(DSP* dsp);


DSP* new_DSP_volume(uint32_t buffer_size)
{
    DSP_volume* volume = xalloc(DSP_volume);
    if (volume == NULL)
    {
        return NULL;
    }
    if (!DSP_init(&volume->parent, del_DSP_volume,
                  DSP_volume_process, buffer_size))
    {
        xfree(volume);
        return NULL;
    }
    Device_register_port(&volume->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&volume->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    return &volume->parent;
}


static void DSP_volume_process(Device* device,
                               uint32_t start,
                               uint32_t until,
                               uint32_t freq,
                               double tempo)
{
    assert(device != NULL);
    assert(freq > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);
    (void)freq;
    (void)tempo;
    DSP_volume* volume = (DSP_volume*)device;
    assert(strcmp(volume->parent.type, "volume") == 0);
    assert(volume->parent.conf != NULL);
    assert(volume->parent.conf->params != NULL);
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    double factor = 1;
    double* dB_arg = Device_params_get_float(volume->parent.conf->params,
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


static void del_DSP_volume(DSP* dsp)
{
    assert(dsp != NULL);
    assert(strcmp(dsp->type, "volume") == 0);
    DSP_volume* volume = (DSP_volume*)dsp;
    Device_uninit(&volume->parent.parent);
    xfree(volume);
    return;
}


