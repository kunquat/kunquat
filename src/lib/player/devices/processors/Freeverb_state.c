

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Freeverb_state.h>

#include <debug/assert.h>
#include <devices/processors/Proc_freeverb.h>
#include <devices/processors/Proc_utils.h>
#include <player/devices/processors/Freeverb_allpass.h>
#include <player/devices/processors/Freeverb_comb.h>
#include <mathnum/common.h>
#include <memory.h>


#define FREEVERB_COMBS 8
#define FREEVERB_ALLPASSES 4


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


typedef struct Freeverb_pstate
{
    Proc_state parent;

    double active_reflect;
    double active_damp;

    Freeverb_comb* comb_left[FREEVERB_COMBS];
    Freeverb_comb* comb_right[FREEVERB_COMBS];
    Freeverb_allpass* allpass_left[FREEVERB_ALLPASSES];
    Freeverb_allpass* allpass_right[FREEVERB_ALLPASSES];
} Freeverb_pstate;


static void Freeverb_pstate_deinit(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dev_state;

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


static void Freeverb_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dstate;
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


static bool Freeverb_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dstate;

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        const uint32_t left_size = max(1, comb_tuning[i] * audio_rate);

        if (!Freeverb_comb_resize_buffer(fstate->comb_left[i], left_size))
            return false;

        const uint32_t right_size =
            max(1, (comb_tuning[i] + stereo_spread) * audio_rate);

        if (!Freeverb_comb_resize_buffer(fstate->comb_right[i], right_size))
            return false;
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        const uint32_t left_size = max(1, allpass_tuning[i] * audio_rate);

        if (!Freeverb_allpass_resize_buffer(fstate->allpass_left[i], left_size))
            return false;

        const uint32_t right_size = max(
                1, (allpass_tuning[i] + stereo_spread) * audio_rate);

        if (!Freeverb_allpass_resize_buffer(fstate->allpass_right[i], right_size))
            return false;
    }

    Freeverb_pstate_reset(dstate);

    return true;
}


static void Freeverb_pstate_clear_history(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    Freeverb_pstate_reset((Device_state*)proc_state);

    return;
}


static void Freeverb_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(tempo > 0);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dstate;

    Proc_freeverb* freeverb = (Proc_freeverb*)dstate->device->dimpl;

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

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float out_l = 0;
        float out_r = 0;
        const float input = (in_data[0][i] + in_data[1][i]) * freeverb->gain;

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

        out_data[0][i] += out_l * freeverb->wet1 + out_r * freeverb->wet2;
        out_data[1][i] += out_r * freeverb->wet1 + out_l * freeverb->wet2;
    }

    return;
}


Device_state* new_Freeverb_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Freeverb_pstate* fstate = memory_alloc_item(Freeverb_pstate);
    if (fstate == NULL)
        return NULL;

    if (!Proc_state_init(&fstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(fstate);
        return NULL;
    }

    fstate->parent.parent.deinit = Freeverb_pstate_deinit;
    fstate->parent.set_audio_rate = Freeverb_pstate_set_audio_rate;
    fstate->parent.reset = Freeverb_pstate_reset;
    fstate->parent.render_mixed = Freeverb_pstate_render_mixed;
    fstate->parent.clear_history = Freeverb_pstate_clear_history;

    const Proc_freeverb* freeverb = (const Proc_freeverb*)device->dimpl;

    fstate->active_reflect = freeverb->reflect_setting;
    fstate->active_damp = freeverb->damp_setting;

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


