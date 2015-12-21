

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_freeverb.h>

#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <devices/processors/Freeverb_allpass.h>
#include <devices/processors/Freeverb_comb.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
    Proc_state parent;

    double active_reflect;
    double active_damp;

    Freeverb_comb* comb_left[FREEVERB_COMBS];
    Freeverb_comb* comb_right[FREEVERB_COMBS];
    Freeverb_allpass* allpass_left[FREEVERB_ALLPASSES];
    Freeverb_allpass* allpass_right[FREEVERB_ALLPASSES];
} Freeverb_state;


static void Freeverb_state_deinit(Device_state* dev_state)
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

    Proc_state_deinit(&fstate->parent.parent);

    return;
}


typedef struct Proc_freeverb
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
} Proc_freeverb;


static void Freeverb_state_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Freeverb_state* fstate = (Freeverb_state*)dstate;
    const Proc_freeverb* freeverb = (const Proc_freeverb*)dstate->device->dimpl;

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

    fstate->active_reflect = freeverb->reflect1;
    fstate->active_damp = freeverb->damp1;

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        assert(fstate->comb_left[i] != NULL);
        assert(fstate->comb_right[i] != NULL);
        Freeverb_comb_set_feedback(fstate->comb_left[i], fstate->active_reflect);
        Freeverb_comb_set_feedback(fstate->comb_right[i], fstate->active_reflect);
        Freeverb_comb_set_damp(fstate->comb_left[i], fstate->active_damp);
        Freeverb_comb_set_damp(fstate->comb_right[i], fstate->active_damp);
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


static bool Proc_freeverb_init(Device_impl* dimpl);

static Device_state* Proc_freeverb_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Proc_freeverb_clear_history(
        const Device_impl* dimpl, Proc_state* proc_state);

static Set_float_func Proc_freeverb_set_refl;
static Set_float_func Proc_freeverb_set_damp;

static bool Proc_freeverb_set_audio_rate(
        const Device_impl* dimpl, Device_state* dstate, int32_t audio_rate);

static void Proc_freeverb_process(
        const Device* device,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void Proc_freeverb_update_reflectivity(Proc_freeverb* freeverb, double reflect);
static void Proc_freeverb_update_damp(Proc_freeverb* freeverb, double damp);
static void Proc_freeverb_update_gain(Proc_freeverb* freeverb, double gain);
static void Proc_freeverb_update_wet(Proc_freeverb* freeverb, double wet);

static void del_Proc_freeverb(Device_impl* dimpl);


Device_impl* new_Proc_freeverb(Processor* proc)
{
    Proc_freeverb* freeverb = memory_alloc_item(Proc_freeverb);
    if (freeverb == NULL)
        return NULL;

    freeverb->parent.device = (Device*)proc;

    Device_impl_register_init(&freeverb->parent, Proc_freeverb_init);
    Device_impl_register_destroy(&freeverb->parent, del_Proc_freeverb);

    return &freeverb->parent;
}


static bool Proc_freeverb_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;

    Device_set_process(freeverb->parent.device, Proc_freeverb_process);

    Device_set_state_creator(freeverb->parent.device, Proc_freeverb_create_state);

    Processor_set_clear_history(
            (Processor*)freeverb->parent.device, Proc_freeverb_clear_history);

    // Register key set/update handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &freeverb->parent,
            "p_f_refl.json",
            initial_reflect,
            Proc_freeverb_set_refl,
            NULL);
    reg_success &= Device_impl_register_set_float(
            &freeverb->parent,
            "p_f_damp.json",
            initial_damp,
            Proc_freeverb_set_damp,
            NULL);

    if (!reg_success)
        return false;

    Device_impl_register_set_audio_rate(
            &freeverb->parent, Proc_freeverb_set_audio_rate);

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

    Proc_freeverb_update_reflectivity(freeverb, initial_reflect);
    Proc_freeverb_update_damp(freeverb, initial_damp);
    const double fixed_gain = 0.015;
    Proc_freeverb_update_gain(freeverb, fixed_gain);
    Proc_freeverb_update_wet(freeverb, initial_wet);

    return true;
}


static Device_state* Proc_freeverb_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Freeverb_state* fstate = memory_alloc_item(Freeverb_state);
    if (fstate == NULL)
        return NULL;

    if (!Proc_state_init(&fstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(fstate);
        return NULL;
    }

    fstate->parent.parent.deinit = Freeverb_state_deinit;
    fstate->parent.reset = Freeverb_state_reset;

    fstate->active_reflect = initial_reflect;
    fstate->active_damp = initial_damp;

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
            del_Device_state(&fstate->parent.parent);
            return NULL;
        }

        const uint32_t right_size = max(
                1, (comb_tuning[i] + stereo_spread) * audio_rate);

        fstate->comb_right[i] = new_Freeverb_comb(right_size);
        if (fstate->comb_right[i] == NULL)
        {
            del_Device_state(&fstate->parent.parent);
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
                del_Device_state(&fstate->parent.parent);
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
                del_Device_state(&fstate->parent.parent);
                return NULL;
            }
        }
    }

    return &fstate->parent.parent;
}


static void Proc_freeverb_clear_history(const Device_impl* dimpl, Proc_state* proc_state)
{
    assert(dimpl != NULL);
    assert(proc_state != NULL);

    Freeverb_state_reset((Device_state*)proc_state);

    return;
}


static bool Proc_freeverb_set_refl(Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;

    if (value > 200)
        value = 200;
    else if (value < 0)
        value = 0;

    Proc_freeverb_update_reflectivity(freeverb, value);

    return true;
}


static bool Proc_freeverb_set_damp(Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;

    if (value > 100)
        value = 100;
    else if (value < 0)
        value = 0;

    Proc_freeverb_update_damp(freeverb, value);

    return true;
}


static bool Proc_freeverb_set_audio_rate(
        const Device_impl* dimpl, Device_state* dstate, int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

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

    Freeverb_state_reset(dstate);

    return true;
}


static void Proc_freeverb_update_reflectivity(Proc_freeverb* freeverb, double reflect)
{
    assert(freeverb != NULL);

    freeverb->reflect_setting = reflect;
    freeverb->reflect = exp2(-5 / freeverb->reflect_setting);
    freeverb->reflect1 = freeverb->reflect;

    return;
}


static void Proc_freeverb_update_damp(Proc_freeverb* freeverb, double damp)
{
    assert(freeverb != NULL);

    freeverb->damp_setting = damp;
    freeverb->damp = freeverb->damp_setting * 0.01;
    freeverb->damp1 = freeverb->damp;

    return;
}


static void Proc_freeverb_update_gain(Proc_freeverb* freeverb, double gain)
{
    assert(freeverb != NULL);

    freeverb->gain = gain;

    return;
}


static void Proc_freeverb_update_wet(Proc_freeverb* freeverb, double wet)
{
    assert(freeverb != NULL);

    static const double scale_wet = 3;
    freeverb->wet = wet * scale_wet;
    freeverb->width = initial_width;

    freeverb->wet1 = freeverb->wet * (freeverb->width * 0.5 + 0.5);
    freeverb->wet2 = freeverb->wet * ((1 - freeverb->width) * 0.5);

    return;
}


static void Proc_freeverb_process(
        const Device* device,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    Freeverb_state* fstate = (Freeverb_state*)Device_states_get_state(
            states, Device_get_id(device));

    Proc_freeverb* freeverb = (Proc_freeverb*)device->dimpl;
    //assert(string_eq(freeverb->parent.type, "freeverb"));
    float* in_data[] = { NULL, NULL };
    float* out_data[] = { NULL, NULL };
    get_raw_input(&fstate->parent.parent, 0, in_data);
    get_raw_output(&fstate->parent.parent, 0, out_data);

    if (fstate->active_reflect != freeverb->reflect1)
    {
        // Update reflectivity settings
        fstate->active_reflect = freeverb->reflect1;

        for (int i = 0; i < FREEVERB_COMBS; ++i)
        {
            Freeverb_comb_set_feedback(fstate->comb_left[i], fstate->active_reflect);
            Freeverb_comb_set_feedback(fstate->comb_right[i], fstate->active_reflect);
        }
    }

    if (fstate->active_damp != freeverb->damp1)
    {
        // Update damp settings
        fstate->active_damp = freeverb->damp1;

        for (int i = 0; i < FREEVERB_COMBS; ++i)
        {
            Freeverb_comb_set_damp(fstate->comb_left[i], fstate->active_damp);
            Freeverb_comb_set_damp(fstate->comb_right[i], fstate->active_damp);
        }
    }

    for (uint32_t i = start; i < until; ++i)
    {
        float out_l = 0;
        float out_r = 0;
        float input = (in_data[0][i] + in_data[1][i]) * freeverb->gain;

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


static void del_Proc_freeverb(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_freeverb* freeverb = (Proc_freeverb*)dimpl;
    memory_free(freeverb);

    return;
}


