

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <Device_impl.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_freeverb.h>
#include <frame.h>
#include <Freeverb_allpass.h>
#include <Freeverb_comb.h>
#include <math_common.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


#define FREEVERB_COMBS 8
#define FREEVERB_ALLPASSES 4


static const double initial_reflect = 20;
static const double initial_damp = 20;
static const double initial_wet = 1 / 3.0; // 3.0 = scale_wet
//static const double initial_dry = 0;
static const double initial_width = 1;

// The following constants are in seconds.
static const double stereo_spread = 0.000521542;
static const double comb_tuning[FREEVERB_COMBS] =
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
static const double allpass_tuning[FREEVERB_ALLPASSES] =
{
    0.012607710,
    0.010000001,
    0.007732427,
    0.005102041,
};


typedef struct Freeverb_state
{
    DSP_state parent;

    Freeverb_comb* comb_left[FREEVERB_COMBS];
    Freeverb_comb* comb_right[FREEVERB_COMBS];
    Freeverb_allpass* allpass_left[FREEVERB_ALLPASSES];
    Freeverb_allpass* allpass_right[FREEVERB_ALLPASSES];
} Freeverb_state;


static void del_Freeverb_state(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Freeverb_state* fstate = (Freeverb_state*)dev_state;

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        del_Freeverb_comb(fstate->comb_left[i]);
        del_Freeverb_comb(fstate->comb_right[i]);
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        del_Freeverb_allpass(fstate->allpass_left[i]);
        del_Freeverb_allpass(fstate->allpass_right[i]);
    }

    return;
}


typedef struct DSP_freeverb
{
    Device_impl parent;

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
} DSP_freeverb;


static void Freeverb_state_reset(
        Freeverb_state* fstate,
        const DSP_freeverb* freeverb)
{
    assert(fstate != NULL);
    assert(freeverb != NULL);

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        Freeverb_comb_clear(fstate->comb_left[i]);
        Freeverb_comb_clear(fstate->comb_right[i]);
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        Freeverb_allpass_clear(fstate->allpass_left[i]);
        Freeverb_allpass_clear(fstate->allpass_right[i]);
    }

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        assert(fstate->comb_left[i] != NULL);
        assert(fstate->comb_right[i] != NULL);
        Freeverb_comb_set_feedback(fstate->comb_left[i], freeverb->reflect1);
        Freeverb_comb_set_feedback(fstate->comb_right[i], freeverb->reflect1);
        Freeverb_comb_set_damp(fstate->comb_left[i], freeverb->damp1);
        Freeverb_comb_set_damp(fstate->comb_right[i], freeverb->damp1);
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        assert(fstate->allpass_left[i] != NULL);
        assert(fstate->allpass_right[i] != NULL);
        Freeverb_allpass_set_feedback(fstate->allpass_left[i], 0.5);
        Freeverb_allpass_set_feedback(fstate->allpass_right[i], 0.5);
    }

    return;
}


static Device_state* DSP_freeverb_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void DSP_freeverb_reset(const Device_impl* dimpl, Device_state* dstate);

static void DSP_freeverb_clear_history(DSP* dsp, DSP_state* dsp_state);
static bool DSP_freeverb_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t mix_rate);
static bool DSP_freeverb_update_key(Device* device, const char* key);

static void DSP_freeverb_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


static void del_DSP_freeverb(Device_impl* dsp_impl);


static void DSP_freeverb_set_reflectivity(
        DSP_freeverb* freeverb,
        double reflect);
static void DSP_freeverb_set_damp(DSP_freeverb* freeverb, double damp);
static void DSP_freeverb_set_gain(DSP_freeverb* freeverb, double gain);
static void DSP_freeverb_set_wet(DSP_freeverb* freeverb, double wet);


Device_impl* new_DSP_freeverb(DSP* dsp, uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    DSP_freeverb* freeverb = memory_alloc_item(DSP_freeverb);
    if (freeverb == NULL)
        return NULL;

    if (!Device_impl_init(&freeverb->parent, del_DSP_freeverb, mix_rate, buffer_size))
    {
        memory_free(freeverb);
        return NULL;
    }

    freeverb->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_freeverb_process);
#if 0
    if (!DSP_init(&freeverb->parent, del_DSP_freeverb,
                  DSP_freeverb_process, buffer_size, mix_rate))
    {
        memory_free(freeverb);
        return NULL;
    }
#endif

    Device_set_state_creator(
            freeverb->parent.device,
            DSP_freeverb_create_state);

    DSP_set_clear_history((DSP*)freeverb->parent.device, DSP_freeverb_clear_history);

    Device_impl_register_reset_device_state(
            &freeverb->parent, DSP_freeverb_reset);

    Device_set_mix_rate_changer(freeverb->parent.device,
                                DSP_freeverb_set_mix_rate);
    Device_set_update_key(freeverb->parent.device, DSP_freeverb_update_key);

    Device_register_port(freeverb->parent.device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(freeverb->parent.device, DEVICE_PORT_TYPE_SEND, 0);

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

#if 0
    if (!DSP_freeverb_set_mix_rate(&freeverb->parent.parent, mix_rate))
    {
        del_DSP(&freeverb->parent);
        return NULL;
    }
#endif

    DSP_freeverb_set_reflectivity(freeverb, initial_reflect);
    DSP_freeverb_set_damp(freeverb, initial_damp);
    const double fixed_gain = 0.015;
    DSP_freeverb_set_gain(freeverb, fixed_gain);
    DSP_freeverb_set_wet(freeverb, initial_wet);

    return &freeverb->parent;
}


static Device_state* DSP_freeverb_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    DSP_freeverb* freeverb = (DSP_freeverb*)device->dimpl;

    Freeverb_state* fstate = memory_alloc_item(Freeverb_state);
    if (fstate == NULL)
        return NULL;

    DSP_state_init(&fstate->parent, device, audio_rate, audio_buffer_size);
    fstate->parent.parent.destroy = del_Freeverb_state;

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        fstate->comb_left[i] = NULL;
        fstate->comb_right[i] = NULL;
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        fstate->allpass_left[i] = NULL;
        fstate->allpass_right[i] = NULL;
    }

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        const uint32_t left_size = MAX(1, comb_tuning[i] * audio_rate);

        fstate->comb_left[i] = new_Freeverb_comb(left_size);
        if (fstate->comb_left[i] == NULL)
        {
            del_Freeverb_state(&fstate->parent.parent);
            return NULL;
        }

        const uint32_t right_size = MAX(
                1, (comb_tuning[i] + stereo_spread) * audio_rate);

        fstate->comb_right[i] = new_Freeverb_comb(right_size);
        if (fstate->comb_right[i] == NULL)
        {
            del_Freeverb_state(&fstate->parent.parent);
            return NULL;
        }
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        const uint32_t left_size = MAX(1, allpass_tuning[i] * audio_rate);

        if (fstate->allpass_left[i] == NULL)
        {
            fstate->allpass_left[i] = new_Freeverb_allpass(left_size);
            if (fstate->allpass_left[i] == NULL)
            {
                del_Freeverb_state(&fstate->parent.parent);
                return NULL;
            }
        }

        const uint32_t right_size = MAX(
                1, (allpass_tuning[i] + stereo_spread) * audio_rate);

        if (fstate->allpass_right[i] == NULL)
        {
            fstate->allpass_right[i] = new_Freeverb_allpass(right_size);
            if (fstate->allpass_right[i] == NULL)
            {
                del_Freeverb_state(&fstate->parent.parent);
                return NULL;
            }
        }
    }

    Freeverb_state_reset(fstate, freeverb);

    return &fstate->parent.parent;
}


static void DSP_freeverb_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    DSP_freeverb* freeverb = (DSP_freeverb*)dimpl;
    DSP_state* dsp_state = (DSP_state*)dstate;

    DSP_freeverb_clear_history((DSP*)freeverb->parent.device, dsp_state);

    return;
}


static void DSP_freeverb_clear_history(DSP* dsp, DSP_state* dsp_state)
{
    assert(dsp != NULL);
    //assert(string_eq(dsp->type, "freeverb"));
    assert(dsp_state != NULL);

    Freeverb_state* fstate = (Freeverb_state*)dsp_state;
    const DSP_freeverb* freeverb = (const DSP_freeverb*)dsp->parent.dimpl;
    Freeverb_state_reset(fstate, freeverb);

    return;
}


static bool DSP_freeverb_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t mix_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(mix_rate > 0);

    const DSP_freeverb* freeverb = (const DSP_freeverb*)device->dimpl;
    Freeverb_state* fstate = (Freeverb_state*)Device_states_get_state(
            dstates, Device_get_id(device));

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        const uint32_t left_size = MAX(1, comb_tuning[i] * mix_rate);

        if (!Freeverb_comb_resize_buffer(fstate->comb_left[i], left_size))
            return false;

        const uint32_t right_size = MAX(
                1, (comb_tuning[i] + stereo_spread) * mix_rate);

        if (!Freeverb_comb_resize_buffer(fstate->comb_right[i], right_size))
            return false;
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        const uint32_t left_size = MAX(1, allpass_tuning[i] * mix_rate);

        if (!Freeverb_allpass_resize_buffer(
                    fstate->allpass_left[i],
                    left_size))
            return false;

        const uint32_t right_size = MAX(
                1, (allpass_tuning[i] + stereo_spread) * mix_rate);

        if (!Freeverb_allpass_resize_buffer(
                    fstate->allpass_right[i],
                    right_size))
            return false;
    }

    Freeverb_state_reset(fstate, freeverb);

    return true;
}


static bool DSP_freeverb_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);

    DSP_freeverb* freeverb = (DSP_freeverb*)device->dimpl;

    if (string_eq(key, "p_refl.jsonf"))
    {
        double* reflect = Device_params_get_float(
                ((DSP*)freeverb->parent.device)->conf->params, key);

        if (reflect == NULL)
        {
            DSP_freeverb_set_reflectivity(freeverb, initial_reflect);
        }
        else if (reflect != NULL)
        {
            if (*reflect > 200)
                *reflect = 200;
            else if (*reflect < 0)
                *reflect = 0;

            DSP_freeverb_set_reflectivity(freeverb, *reflect);
        }
    }
    else if (string_eq(key, "p_damp.jsonf"))
    {
        double* damp = Device_params_get_float(
                ((DSP*)freeverb->parent.device)->conf->params, key);

        if (damp == NULL)
        {
            DSP_freeverb_set_damp(freeverb, initial_damp);
        }
        else if (damp != NULL && freeverb->damp_setting != *damp)
        {
            if (*damp > 100)
                *damp = 100;
            else if (*damp < 0)
                *damp = 0;

            DSP_freeverb_set_damp(freeverb, *damp);
        }
    }

    return true;
}


static void DSP_freeverb_set_reflectivity(
        DSP_freeverb* freeverb,
        double reflect)
{
    assert(freeverb != NULL);

    freeverb->reflect_setting = reflect;
    freeverb->reflect = exp2(-5 / freeverb->reflect_setting);
    freeverb->reflect1 = freeverb->reflect;

    return;
}


static void DSP_freeverb_set_damp(DSP_freeverb* freeverb, double damp)
{
    assert(freeverb != NULL);

    freeverb->damp_setting = damp;
    freeverb->damp = freeverb->damp_setting * 0.01;
    freeverb->damp1 = freeverb->damp;

    return;
}


static void DSP_freeverb_set_gain(DSP_freeverb* freeverb, double gain)
{
    assert(freeverb != NULL);

    freeverb->gain = gain;

    return;
}


static void DSP_freeverb_set_wet(DSP_freeverb* freeverb, double wet)
{
    assert(freeverb != NULL);

    static const double scale_wet = 3;
    freeverb->wet = wet * scale_wet;
    freeverb->width = initial_width;

    freeverb->wet1 = freeverb->wet * (freeverb->width * 0.5 + 0.5);
    freeverb->wet2 = freeverb->wet * ((1 - freeverb->width) * 0.5);

    return;
}


#if 0
static void DSP_freeverb_check_params(
        DSP_freeverb* freeverb,
        Freeverb_state* fstate)
{
    assert(freeverb != NULL);
    assert(freeverb->parent.conf != NULL);
    assert(freeverb->parent.conf->params != NULL);
    assert(fstate != NULL);

#if 0
    double* width = Device_params_get_float(
            freeverb->parent.conf->params, "p_width.jsonf");
    if (width == NULL && freeverb->width != initial_width)
        DSP_freeverb_set_width(freeverb, fstate, initial_width);
    else if (width != NULL && freeverb->width != *width)
        DSP_freeverb_set_width(freeverb, fstate, *width);
#endif

    return;
}
#endif


static void DSP_freeverb_process(
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

    Freeverb_state* fstate = (Freeverb_state*)Device_states_get_state(
            states, Device_get_id(device));

    DSP_freeverb* freeverb = (DSP_freeverb*)device->dimpl;
    //assert(string_eq(freeverb->parent.type, "freeverb"));
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(&fstate->parent.parent, 0, in_data);
    DSP_get_raw_output(&fstate->parent.parent, 0, out_data);

    for (uint32_t i = start; i < until; ++i)
    {
        kqt_frame out_l = 0;
        kqt_frame out_r = 0;
        kqt_frame input = (in_data[0][i] + in_data[1][i]) * freeverb->gain;

        for (int comb = 0; comb < FREEVERB_COMBS; ++comb)
        {
            out_l += Freeverb_comb_process(fstate->comb_left[comb], input);
            out_r += Freeverb_comb_process(fstate->comb_right[comb], input);
        }

        for (int allpass = 0; allpass < FREEVERB_ALLPASSES; ++allpass)
        {
            out_l = Freeverb_allpass_process(
                    fstate->allpass_left[allpass], out_l);
            out_r = Freeverb_allpass_process(
                    fstate->allpass_right[allpass], out_r);
        }

        out_data[0][i] += out_l * freeverb->wet1 + out_r * freeverb->wet2
                                  /* + in_data[0][i] * freeverb->dry */;
        out_data[1][i] += out_r * freeverb->wet1 + out_l * freeverb->wet2
                                  /* + in_data[1][i] * freeverb->dry */;
    }

    return;
}


static void del_DSP_freeverb(Device_impl* dsp_impl)
{
    if (dsp_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "freeverb"));
    DSP_freeverb* freeverb = (DSP_freeverb*)dsp_impl;

    memory_free(freeverb);

    return;
}


