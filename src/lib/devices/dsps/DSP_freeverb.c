

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/DSP.h>
#include <devices/dsps/DSP_common.h>
#include <devices/dsps/DSP_freeverb.h>
#include <devices/dsps/Freeverb_allpass.h>
#include <devices/dsps/Freeverb_comb.h>
#include <frame.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>


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

static bool DSP_freeverb_set_refl(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value);

static bool DSP_freeverb_set_damp(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value);

static void DSP_freeverb_clear_history(
        const Device_impl* dimpl, DSP_state* dsp_state);

static bool DSP_freeverb_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate);

static void DSP_freeverb_process(
        const Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


static void del_DSP_freeverb(Device_impl* dsp_impl);


static void DSP_freeverb_update_reflectivity(
        DSP_freeverb* freeverb,
        double reflect);
static void DSP_freeverb_update_damp(DSP_freeverb* freeverb, double damp);
static void DSP_freeverb_update_gain(DSP_freeverb* freeverb, double gain);
static void DSP_freeverb_update_wet(DSP_freeverb* freeverb, double wet);


Device_impl* new_DSP_freeverb(DSP* dsp)
{
    DSP_freeverb* freeverb = memory_alloc_item(DSP_freeverb);
    if (freeverb == NULL)
        return NULL;

    if (!Device_impl_init(&freeverb->parent, del_DSP_freeverb))
    {
        memory_free(freeverb);
        return NULL;
    }

    freeverb->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_freeverb_process);

    Device_set_state_creator(
            freeverb->parent.device,
            DSP_freeverb_create_state);

    DSP_set_clear_history((DSP*)freeverb->parent.device, DSP_freeverb_clear_history);

    Device_impl_register_reset_device_state(
            &freeverb->parent, DSP_freeverb_reset);

    // Register key set/update handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &freeverb->parent,
            "p_f_refl.json",
            initial_reflect,
            DSP_freeverb_set_refl,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &freeverb->parent,
            "p_f_damp.json",
            initial_damp,
            DSP_freeverb_set_damp,
            NULL);

    Device_impl_register_set_audio_rate(
            &freeverb->parent, DSP_freeverb_set_audio_rate);

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

    DSP_freeverb_update_reflectivity(freeverb, initial_reflect);
    DSP_freeverb_update_damp(freeverb, initial_damp);
    const double fixed_gain = 0.015;
    DSP_freeverb_update_gain(freeverb, fixed_gain);
    DSP_freeverb_update_wet(freeverb, initial_wet);

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
        const uint32_t left_size = max(1, comb_tuning[i] * audio_rate);

        fstate->comb_left[i] = new_Freeverb_comb(left_size);
        if (fstate->comb_left[i] == NULL)
        {
            del_Freeverb_state(&fstate->parent.parent);
            return NULL;
        }

        const uint32_t right_size = max(
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
        const uint32_t left_size = max(1, allpass_tuning[i] * audio_rate);

        if (fstate->allpass_left[i] == NULL)
        {
            fstate->allpass_left[i] = new_Freeverb_allpass(left_size);
            if (fstate->allpass_left[i] == NULL)
            {
                del_Freeverb_state(&fstate->parent.parent);
                return NULL;
            }
        }

        const uint32_t right_size = max(
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

    DSP_state* dsp_state = (DSP_state*)dstate;
    DSP_freeverb_clear_history(dimpl, dsp_state);

    return;
}


static void DSP_freeverb_clear_history(
        const Device_impl* dimpl, DSP_state* dsp_state)
{
    assert(dimpl != NULL);
    //assert(string_eq(dsp->type, "freeverb"));
    assert(dsp_state != NULL);

    Freeverb_state* fstate = (Freeverb_state*)dsp_state;
    const DSP_freeverb* freeverb = (const DSP_freeverb*)dimpl;
    Freeverb_state_reset(fstate, freeverb);

    return;
}


static bool DSP_freeverb_set_refl(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_freeverb* freeverb = (DSP_freeverb*)dimpl;

    if (value > 200)
        value = 200;
    else if (value < 0)
        value = 0;

    DSP_freeverb_update_reflectivity(freeverb, value);

    return true;
}


static bool DSP_freeverb_set_damp(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_freeverb* freeverb = (DSP_freeverb*)dimpl;

    if (value > 100)
        value = 100;
    else if (value < 0)
        value = 0;

    DSP_freeverb_update_damp(freeverb, value);

    return true;
}


static bool DSP_freeverb_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

    const DSP_freeverb* freeverb = (const DSP_freeverb*)dimpl;
    Freeverb_state* fstate = (Freeverb_state*)dstate;

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        const uint32_t left_size = max(1, comb_tuning[i] * audio_rate);

        if (!Freeverb_comb_resize_buffer(fstate->comb_left[i], left_size))
            return false;

        const uint32_t right_size = max(
                1, (comb_tuning[i] + stereo_spread) * audio_rate);

        if (!Freeverb_comb_resize_buffer(fstate->comb_right[i], right_size))
            return false;
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        const uint32_t left_size = max(1, allpass_tuning[i] * audio_rate);

        if (!Freeverb_allpass_resize_buffer(
                    fstate->allpass_left[i],
                    left_size))
            return false;

        const uint32_t right_size = max(
                1, (allpass_tuning[i] + stereo_spread) * audio_rate);

        if (!Freeverb_allpass_resize_buffer(
                    fstate->allpass_right[i],
                    right_size))
            return false;
    }

    Freeverb_state_reset(fstate, freeverb);

    return true;
}


static void DSP_freeverb_update_reflectivity(
        DSP_freeverb* freeverb,
        double reflect)
{
    assert(freeverb != NULL);

    freeverb->reflect_setting = reflect;
    freeverb->reflect = exp2(-5 / freeverb->reflect_setting);
    freeverb->reflect1 = freeverb->reflect;

    return;
}


static void DSP_freeverb_update_damp(DSP_freeverb* freeverb, double damp)
{
    assert(freeverb != NULL);

    freeverb->damp_setting = damp;
    freeverb->damp = freeverb->damp_setting * 0.01;
    freeverb->damp1 = freeverb->damp;

    return;
}


static void DSP_freeverb_update_gain(DSP_freeverb* freeverb, double gain)
{
    assert(freeverb != NULL);

    freeverb->gain = gain;

    return;
}


static void DSP_freeverb_update_wet(DSP_freeverb* freeverb, double wet)
{
    assert(freeverb != NULL);

    static const double scale_wet = 3;
    freeverb->wet = wet * scale_wet;
    freeverb->width = initial_width;

    freeverb->wet1 = freeverb->wet * (freeverb->width * 0.5 + 0.5);
    freeverb->wet2 = freeverb->wet * ((1 - freeverb->width) * 0.5);

    return;
}


static void DSP_freeverb_process(
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
    (void)freq;
    (void)tempo;

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


