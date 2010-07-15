

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
#include <xassert.h>
#include <xmemory.h>


#define FREEVERB_COMBS 8
#define FREEVERB_ALLPASSES 4


const double muted = 0;
const double fixed_gain = 0.015;
const double scale_wet = 3;
const double scale_dry = 2;
const double scale_damp = 0.4;
const double scale_room = 0.28;
const double offset_room = 0.7;
const double initial_room = 0.5;
const double initial_damp = 0.5;
const double initial_wet = 1 / 3.0; // scale_wet
const double initial_dry = 0;
const double initial_width = 1;
const double initial_mode = 0;
const double freeze_mode = 0.5;


// The following constants are in seconds.
const double stereo_spread = 0.000521542;
const double comb_tuning[] =
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
const double allpass_tuning[] =
{
    0.012607710,
    0.010000001,
    0.007732427,
    0.005102041,
};


typedef struct DSP_freeverb
{
    DSP parent;
    kqt_frame gain;
    kqt_frame room_size;
    kqt_frame room_size1;
    kqt_frame damp;
    kqt_frame damp1;
    kqt_frame wet;
    kqt_frame wet1;
    kqt_frame wet2;
    kqt_frame dry;
    kqt_frame width;
    kqt_frame mode;
    Freeverb_comb* comb_left[FREEVERB_COMBS];
    Freeverb_comb* comb_right[FREEVERB_COMBS];
    Freeverb_allpass* allpass_left[FREEVERB_ALLPASSES];
    Freeverb_allpass* allpass_right[FREEVERB_ALLPASSES];
} DSP_freeverb;


static bool DSP_freeverb_set_mix_rate(Device* device, uint32_t mix_rate);

static void DSP_freeverb_update(DSP_freeverb* freeverb);

static void DSP_freeverb_set_room_size(DSP_freeverb* freeverb,
                                       kqt_frame room_size);
static void DSP_freeverb_set_damp(DSP_freeverb* freeverb, kqt_frame damp);
static void DSP_freeverb_set_wet(DSP_freeverb* freeverb, kqt_frame wet);
static void DSP_freeverb_set_dry(DSP_freeverb* freeverb, kqt_frame dry);
static void DSP_freeverb_set_width(DSP_freeverb* freeverb, kqt_frame width);
static void DSP_freeverb_set_mode(DSP_freeverb* freeverb, kqt_frame mode);

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
    if (!DSP_freeverb_set_mix_rate(&freeverb->parent.parent, mix_rate))
    {
        del_DSP(&freeverb->parent);
        return NULL;
    }
    DSP_freeverb_set_wet(freeverb, initial_wet);
    DSP_freeverb_set_room_size(freeverb, initial_room);
    DSP_freeverb_set_dry(freeverb, initial_dry);
    DSP_freeverb_set_damp(freeverb, initial_damp);
    DSP_freeverb_set_width(freeverb, initial_width);
    DSP_freeverb_set_mode(freeverb, initial_mode);
    return &freeverb->parent;
}


static bool DSP_freeverb_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    DSP_freeverb* freeverb = (DSP_freeverb*)device;
    assert(strcmp(freeverb->parent.type, "freeverb") == 0);
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
    if (freeverb->mode >= freeze_mode)
    {
        freeverb->room_size1 = 1;
        freeverb->damp1 = 0;
        freeverb->gain = muted;
    }
    else
    {
        freeverb->room_size1 = freeverb->room_size;
        freeverb->damp1 = freeverb->damp;
        freeverb->gain = fixed_gain;
    }
    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        assert(freeverb->comb_left[i] != NULL);
        assert(freeverb->comb_right[i] != NULL);
        Freeverb_comb_set_feedback(freeverb->comb_left[i],
                                   freeverb->room_size1);
        Freeverb_comb_set_feedback(freeverb->comb_right[i],
                                   freeverb->room_size1);
        Freeverb_comb_set_damp(freeverb->comb_left[i],
                               freeverb->damp1);
        Freeverb_comb_set_damp(freeverb->comb_right[i],
                               freeverb->damp1);
    }
    return;
}


static void DSP_freeverb_set_room_size(DSP_freeverb* freeverb,
                                       kqt_frame room_size)
{
    assert(freeverb != NULL);
    freeverb->room_size = (room_size * scale_room) + offset_room;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_set_damp(DSP_freeverb* freeverb, kqt_frame damp)
{
    assert(freeverb != NULL);
    freeverb->damp = damp * scale_damp;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_set_wet(DSP_freeverb* freeverb, kqt_frame wet)
{
    assert(freeverb != NULL);
    freeverb->wet = wet * scale_wet;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_set_dry(DSP_freeverb* freeverb, kqt_frame dry)
{
    assert(freeverb != NULL);
    freeverb->dry = dry * scale_dry;
    return;
}


static void DSP_freeverb_set_width(DSP_freeverb* freeverb, kqt_frame width)
{
    assert(freeverb != NULL);
    freeverb->width = width;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_set_mode(DSP_freeverb* freeverb, kqt_frame mode)
{
    assert(freeverb != NULL);
    freeverb->mode = mode;
    DSP_freeverb_update(freeverb);
    return;
}


static void DSP_freeverb_process(Device* device,
                                 uint32_t start,
                                 uint32_t until,
                                 uint32_t freq,
                                 double tempo)
{
    (void)start;
    (void)until;
    assert(false);
    assert(device != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    (void)tempo;
    DSP_freeverb* freeverb = (DSP_freeverb*)device;
    assert(strcmp(freeverb->parent.type, "freeverb") == 0);
    assert(freeverb->parent.conf != NULL);
    assert(freeverb->parent.conf->params != NULL);
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    assert(false);
    return;
}


static void del_DSP_freeverb(DSP* dsp)
{
    assert(dsp != NULL);
    assert(strcmp(dsp->type, "freeverb") == 0);
    DSP_freeverb* freeverb = (DSP_freeverb*)dsp;
    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        if (freeverb->comb_left[i] != NULL)
        {
            del_Freeverb_comb(freeverb->comb_left[i]);
        }
        if (freeverb->comb_right[i] != NULL)
        {
            del_Freeverb_comb(freeverb->comb_right[i]);
        }
    }
    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        if (freeverb->allpass_left[i] != NULL)
        {
            del_Freeverb_allpass(freeverb->allpass_left[i]);
        }
        if (freeverb->allpass_right[i] != NULL)
        {
            del_Freeverb_allpass(freeverb->allpass_right[i]);
        }
    }
    Device_uninit(&freeverb->parent.parent);
    xfree(freeverb);
    return;
}


