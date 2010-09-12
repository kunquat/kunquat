

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
#include <DSP_freeverb.h>
#include <frame.h>
#include <Freeverb_allpass.h>
#include <Freeverb_comb.h>
#include <math_common.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


#define FREEVERB_COMBS 8
#define FREEVERB_ALLPASSES 4


const double initial_reflect = 20;
const double initial_damp = 20;
const double initial_wet = 1 / 3.0; // 3.0 = scale_wet
//const double initial_dry = 0;
const double initial_width = 1;


typedef struct DSP_freeverb
{
    DSP parent;
    double gain;
    double reflect;
    double reflect1;
    double damp;
    double damp1;
    double wet;
    double wet1;
    double wet2;
//    double dry;
    double width;
    double reflect_setting;
    double damp_setting;
    Freeverb_comb* comb_left[FREEVERB_COMBS];
    Freeverb_comb* comb_right[FREEVERB_COMBS];
    Freeverb_allpass* allpass_left[FREEVERB_ALLPASSES];
    Freeverb_allpass* allpass_right[FREEVERB_ALLPASSES];
} DSP_freeverb;


static void DSP_freeverb_reset(Device* device);
static bool DSP_freeverb_set_mix_rate(Device* device, uint32_t mix_rate);

static void DSP_freeverb_update(DSP_freeverb* freeverb);

static void DSP_freeverb_set_reflectivity(DSP_freeverb* freeverb,
                                          double reflect);
static void DSP_freeverb_set_damp(DSP_freeverb* freeverb, double damp);
static void DSP_freeverb_set_wet(DSP_freeverb* freeverb, double wet);
//static void DSP_freeverb_set_dry(DSP_freeverb* freeverb, double dry);
static void DSP_freeverb_set_width(DSP_freeverb* freeverb, double width);

static void DSP_freeverb_check_params(DSP_freeverb* freeverb);

static void DSP_freeverb_process(Device* device,
                                 uint32_t start,
                                 uint32_t until,
                                 uint32_t freq,
                                 double tempo);


static void del_DSP_freeverb(DSP* dsp);


DSP* new_DSP_freeverb(uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    DSP_freeverb* freeverb = xalloc(DSP_freeverb);
    if (freeverb == NULL)
    {
        return NULL;
    }
    if (!DSP_init(&freeverb->parent, del_DSP_freeverb,
                  DSP_freeverb_process, buffer_size, mix_rate))
    {
        xfree(freeverb);
        return NULL;
    }
    Device_set_mix_rate_changer(&freeverb->parent.parent,
                                DSP_freeverb_set_mix_rate);
    Device_set_reset(&freeverb->parent.parent, DSP_freeverb_reset);
    Device_register_port(&freeverb->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&freeverb->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        freeverb->comb_left[i] = NULL;
        freeverb->comb_right[i] = NULL;
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        freeverb->allpass_left[i] = NULL;
        freeverb->allpass_right[i] = NULL;
    }
    freeverb->gain = 0;
    freeverb->reflect = 0;
    freeverb->reflect1 = 0;
    freeverb->damp = 0;
    freeverb->damp1 = 0;
    freeverb->wet = 0;
    freeverb->wet1 = 0;
    freeverb->wet2 = 0;
//    freeverb->dry = 0;
    freeverb->width = 0;
    if (!DSP_freeverb_set_mix_rate(&freeverb->parent.parent, mix_rate))
    {
        del_DSP(&freeverb->parent);
        return NULL;
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        assert(freeverb->allpass_left[i] != NULL);
        assert(freeverb->allpass_right[i] != NULL);
        Freeverb_allpass_set_feedback(freeverb->allpass_left[i], 0.5);
        Freeverb_allpass_set_feedback(freeverb->allpass_right[i], 0.5);
    }
    DSP_freeverb_set_wet(freeverb, initial_wet);
    DSP_freeverb_set_reflectivity(freeverb, initial_reflect);
//    DSP_freeverb_set_dry(freeverb, initial_dry);
    DSP_freeverb_set_damp(freeverb, initial_damp);
    DSP_freeverb_set_width(freeverb, initial_width);
    return &freeverb->parent;
}


static void DSP_freeverb_reset(Device* device)
{
    assert(device != NULL);
    DSP_reset(device);
    DSP_freeverb* freeverb = (DSP_freeverb*)device;
    if (freeverb->comb_left[0] == NULL)
    {
        return;
    }
    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        Freeverb_comb_clear(freeverb->comb_left[i]);
        Freeverb_comb_clear(freeverb->comb_right[i]);
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        Freeverb_allpass_clear(freeverb->allpass_left[i]);
        Freeverb_allpass_clear(freeverb->allpass_right[i]);
    }
    return;
}


static bool DSP_freeverb_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    DSP_freeverb* freeverb = (DSP_freeverb*)device;
    // The following constants are in seconds.
    const double stereo_spread = 0.000521542;
    const double comb_tuning[FREEVERB_COMBS] =
    {
        0.025306123,
        0.026938776,
        0.028956917,
        0.030748300,
        0.032244898,
        0.033809524,
        0.035306123,
        0.036666667,
    };
    const double allpass_tuning[FREEVERB_ALLPASSES] =
    {
        0.012607710,
        0.010000001,
        0.007732427,
        0.005102041,
    };

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        uint32_t left_size = MAX(1, comb_tuning[i] * mix_rate);
        if (freeverb->comb_left[i] == NULL)
        {
            freeverb->comb_left[i] = new_Freeverb_comb(left_size);
            if (freeverb->comb_left[i] == NULL)
            {
                return false;
            }
        }
        else if (!Freeverb_comb_resize_buffer(freeverb->comb_left[i],
                                              left_size))
        {
            return false;
        }
        uint32_t right_size = MAX(1, (comb_tuning[i] + stereo_spread) *
                                     mix_rate);
        if (freeverb->comb_right[i] == NULL)
        {
            freeverb->comb_right[i] = new_Freeverb_comb(right_size);
            if (freeverb->comb_right[i] == NULL)
            {
                return false;
            }
        }
        else if (!Freeverb_comb_resize_buffer(freeverb->comb_right[i],
                                              right_size))
        {
            return false;
        }
    }
    
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        uint32_t left_size = MAX(1, allpass_tuning[i] * mix_rate);
        if (freeverb->allpass_left[i] == NULL)
        {
            freeverb->allpass_left[i] = new_Freeverb_allpass(left_size);
            if (freeverb->allpass_left[i] == NULL)
            {
                return false;
            }
        }
        else if (!Freeverb_allpass_resize_buffer(freeverb->allpass_left[i],
                                                 left_size))
        {
            return false;
        }
        uint32_t right_size = MAX(1, (allpass_tuning[i] + stereo_spread) *
                                     mix_rate);
        if (freeverb->allpass_right[i] == NULL)
        {
            freeverb->allpass_right[i] = new_Freeverb_allpass(right_size);
            if (freeverb->allpass_right[i] == NULL)
            {
                return false;
            }
        }
        else if (!Freeverb_allpass_resize_buffer(freeverb->allpass_right[i],
                                                 right_size))
        {
            return false;
        }
    }
    return true;
}


static void DSP_freeverb_update(DSP_freeverb* freeverb)
{
    assert(freeverb != NULL);
    freeverb->wet1 = freeverb->wet * (freeverb->width / 2 + 0.5);
    freeverb->wet2 = freeverb->wet * ((1 - freeverb->width) / 2);
    freeverb->reflect1 = freeverb->reflect;
    freeverb->damp1 = freeverb->damp;
    const double fixed_gain = 0.015;
    freeverb->gain = fixed_gain;
    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        assert(freeverb->comb_left[i] != NULL);
        assert(freeverb->comb_right[i] != NULL);
        Freeverb_comb_set_feedback(freeverb->comb_left[i],
                                   freeverb->reflect1);
        Freeverb_comb_set_feedback(freeverb->comb_right[i],
                                   freeverb->reflect1);
        Freeverb_comb_set_damp(freeverb->comb_left[i],
                               freeverb->damp1);
        Freeverb_comb_set_damp(freeverb->comb_right[i],
                               freeverb->damp1);
    }
    return;
}


static void DSP_freeverb_set_reflectivity(DSP_freeverb* freeverb,
                                          double reflect)
{
    assert(freeverb != NULL);
    freeverb->reflect_setting = reflect;
    freeverb->reflect = exp2(-5 / reflect);
//    freeverb->reflect = pow(reflect, 1.0 / 8) * 0.98;
//    const double scale_room = 0.28;
//    const double offset_room = 0.7;
//    freeverb->room_size = (room_size * scale_room) + offset_room;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_set_damp(DSP_freeverb* freeverb, double damp)
{
    assert(freeverb != NULL);
    freeverb->damp_setting = damp;
//    const double scale_damp = 0.4;
    freeverb->damp = damp / 100;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_set_wet(DSP_freeverb* freeverb, double wet)
{
    assert(freeverb != NULL);
    const double scale_wet = 3;
    freeverb->wet = wet * scale_wet;
    DSP_freeverb_update(freeverb);
    return;
}


#if 0
static void DSP_freeverb_set_dry(DSP_freeverb* freeverb, double dry)
{
    assert(freeverb != NULL);
    const double scale_dry = 2;
    freeverb->dry = dry * scale_dry;
    return;
}
#endif


static void DSP_freeverb_set_width(DSP_freeverb* freeverb, double width)
{
    assert(freeverb != NULL);
    freeverb->width = width;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_check_params(DSP_freeverb* freeverb)
{
    assert(freeverb != NULL);
    assert(freeverb->parent.conf != NULL);
    assert(freeverb->parent.conf->params != NULL);
    double* reflect = Device_params_get_float(freeverb->parent.conf->params,
                                              "p_reflectivity.jsonf");
    if (reflect == NULL && freeverb->reflect_setting != initial_reflect)
    {
        DSP_freeverb_set_reflectivity(freeverb, initial_reflect);
    }
    else if (reflect != NULL && freeverb->reflect_setting != *reflect)
    {
        if (*reflect > 200)
        {
            *reflect = 200;
        }
        else if (*reflect < 0)
        {
            *reflect = 0;
        }
        DSP_freeverb_set_reflectivity(freeverb, *reflect);
    }
    double* damp = Device_params_get_float(freeverb->parent.conf->params,
                                           "p_damp.jsonf");
    if (damp == NULL && freeverb->damp_setting != initial_damp)
    {
        DSP_freeverb_set_damp(freeverb, initial_damp);
    }
    else if (damp != NULL && freeverb->damp_setting != *damp)
    {
        if (*damp > 100)
        {
            *damp = 100;
        }
        else if (*damp < 0)
        {
            *damp = 0;
        }
        DSP_freeverb_set_damp(freeverb, *damp);
    }
#if 0
    double* width = Device_params_get_float(freeverb->parent.conf->params,
                                            "p_width.jsonf");
    if (width == NULL && freeverb->width != initial_width)
    {
        DSP_freeverb_set_width(freeverb, initial_width);
    }
    else if (width != NULL && freeverb->width != *width)
    {
        DSP_freeverb_set_width(freeverb, *width);
    }
#endif
    return;
}


static void DSP_freeverb_process(Device* device,
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
    DSP_freeverb* freeverb = (DSP_freeverb*)device;
    assert(string_eq(freeverb->parent.type, "freeverb"));
    DSP_freeverb_check_params(freeverb);
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    for (uint32_t i = start; i < until; ++i)
    {
        kqt_frame out_l = 0;
        kqt_frame out_r = 0;
        kqt_frame input = (in_data[0][i] + in_data[1][i]) * freeverb->gain;
        for (int comb = 0; comb < FREEVERB_COMBS; ++comb)
        {
            out_l += Freeverb_comb_process(freeverb->comb_left[comb], input);
            out_r += Freeverb_comb_process(freeverb->comb_right[comb], input);
        }
        for (int allpass = 0; allpass < FREEVERB_ALLPASSES; ++allpass)
        {
            out_l = Freeverb_allpass_process(freeverb->allpass_left[allpass],
                                             out_l);
            out_r = Freeverb_allpass_process(freeverb->allpass_right[allpass],
                                             out_r);
        }
        out_data[0][i] += out_l * freeverb->wet1 + out_r * freeverb->wet2
                                  /* + in_data[0][i] * freeverb->dry */;
        out_data[1][i] += out_r * freeverb->wet1 + out_l * freeverb->wet2
                                  /* + in_data[1][i] * freeverb->dry */;
    }
    return;
}


static void del_DSP_freeverb(DSP* dsp)
{
    if (dsp == NULL)
    {
        return;
    }
    assert(string_eq(dsp->type, "freeverb"));
    DSP_freeverb* freeverb = (DSP_freeverb*)dsp;
    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        del_Freeverb_comb(freeverb->comb_left[i]);
        del_Freeverb_comb(freeverb->comb_right[i]);
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        del_Freeverb_allpass(freeverb->allpass_left[i]);
        del_Freeverb_allpass(freeverb->allpass_right[i]);
    }
    xfree(freeverb);
    return;
}


