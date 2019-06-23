

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
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
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_freeverb.h>
#include <intrinsics.h>
#include <mathnum/common.h>
#include <mathnum/fast_exp2.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Freeverb_allpass.h>
#include <player/devices/processors/Freeverb_comb.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>


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

    Freeverb_comb* combs[2][FREEVERB_COMBS];
    Freeverb_allpass* allpasses[2][FREEVERB_ALLPASSES];
} Freeverb_pstate;


static void del_Freeverb_pstate(Device_state* dstate)
{
    rassert(dstate != NULL);

    Freeverb_pstate* fpstate = (Freeverb_pstate*)dstate;

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < FREEVERB_COMBS; ++i)
            del_Freeverb_comb(fpstate->combs[ch][i]);

        for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
            del_Freeverb_allpass(fpstate->allpasses[ch][i]);
    }

    memory_free(fpstate);

    return;
}


static void Freeverb_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dstate;

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < FREEVERB_COMBS; ++i)
        {
            Freeverb_comb_clear(fstate->combs[ch][i]);
        }

        for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
        {
            Freeverb_allpass_clear(fstate->allpasses[ch][i]);
            Freeverb_allpass_set_feedback(fstate->allpasses[ch][i], 0.5);
        }
    }

    return;
}


static bool Freeverb_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    rassert(dstate != NULL);
    rassert(audio_rate > 0);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dstate;

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        const int32_t left_size = (int32_t)max(1, comb_tuning[i] * audio_rate);

        if (!Freeverb_comb_resize_buffer(fstate->combs[0][i], left_size))
            return false;

        const int32_t right_size =
            (int32_t)max(1, (comb_tuning[i] + stereo_spread) * audio_rate);

        if (!Freeverb_comb_resize_buffer(fstate->combs[1][i], right_size))
            return false;
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        const int32_t left_size = (int32_t)max(1, allpass_tuning[i] * audio_rate);

        if (!Freeverb_allpass_resize_buffer(fstate->allpasses[0][i], left_size))
            return false;

        const int32_t right_size =
            (int32_t)max(1, (allpass_tuning[i] + stereo_spread) * audio_rate);

        if (!Freeverb_allpass_resize_buffer(fstate->allpasses[1][i], right_size))
            return false;
    }

    Freeverb_pstate_reset(dstate);

    return true;
}


static void Freeverb_pstate_clear_history(Proc_state* proc_state)
{
    rassert(proc_state != NULL);

    Freeverb_pstate_reset((Device_state*)proc_state);

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_REFL,
    PORT_IN_DAMP,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int FREEVERB_WB_FIXED_REFL = WORK_BUFFER_IMPL_1;
static const int FREEVERB_WB_FIXED_DAMP = WORK_BUFFER_IMPL_2;
static const int FREEVERB_WB_COMB_INPUT = WORK_BUFFER_IMPL_3;


static void Freeverb_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(tempo > 0);

    Freeverb_pstate* fstate = (Freeverb_pstate*)dstate;

    Proc_freeverb* freeverb = (Proc_freeverb*)dstate->device->dimpl;

    // Get reflectivity parameter stream
    Work_buffer* refls_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_REFL);
    float* refls = Work_buffer_is_valid(refls_wb)
        ? Work_buffer_get_contents_mut(refls_wb) : NULL;
    if (refls == NULL)
    {
        refls = Work_buffers_get_buffer_contents_mut(wbs, FREEVERB_WB_FIXED_REFL);
        const float fixed_refl = (float)exp2(-5 / freeverb->reflect_setting);
        for (int32_t i = 0; i < frame_count; ++i)
            refls[i] = fixed_refl;
    }
    else
    {
        // Convert reflectivity to the domain of our algorithm
        const double max_param_inv = -5.0 / 200.0;
        const double min_param_inv = -5.0 / 0.001;
        for (int32_t i = 0; i < frame_count; ++i)
        {
            const double orig_refl = refls[i];
            const double param_inv = -5.0 / max(0, orig_refl);
            const float refl =
                (float)fast_exp2(clamp(param_inv, min_param_inv, max_param_inv));
            refls[i] = refl;
        }
    }

    // Get damp parameter stream
    const float damp_adjust = 44100 / (float)Device_state_get_audio_rate(dstate);
    Work_buffer* damps_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_DAMP);
    float* damps = NULL;
    if (!Work_buffer_is_valid(damps_wb))
    {
        damps = Work_buffers_get_buffer_contents_mut(wbs, FREEVERB_WB_FIXED_DAMP);
        const float adj_damp =
            powf((float)(freeverb->damp_setting * 0.01f), damp_adjust);
        const float clamped = clamp(adj_damp, 0, 1);
        for (int32_t i = 0; i < frame_count; ++i)
            damps[i] = clamped;
    }
    else
    {
        const int32_t const_start = Work_buffer_get_const_start(damps_wb);
        const int32_t var_stop = min(const_start, frame_count);

        damps = Work_buffer_get_contents_mut(damps_wb);

        for (int32_t i = 0; i < var_stop; ++i)
        {
            const float adj_damp = powf(damps[i] * 0.01f, damp_adjust);
            damps[i] = clamp(adj_damp, 0, 1);
        }

        if (var_stop < frame_count)
        {
            const float adj_damp = powf(damps[var_stop] * 0.01f, damp_adjust);
            const float clamped = clamp(adj_damp, 0, 1);

            for (int32_t i = var_stop; i < frame_count; ++i)
                damps[i] = clamped;
        }
    }

    Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        in_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);
        if (!Work_buffer_is_valid(in_wbs[ch]))
            in_wbs[ch] = NULL;
    }

    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);

    // Apply reverb
    {
        Work_buffer* comb_input_wb =
            Work_buffers_get_buffer_mut(wbs, FREEVERB_WB_COMB_INPUT);
        float* comb_input = Work_buffer_get_contents_mut(comb_input_wb);

        if ((in_wbs[0] != NULL) && (in_wbs[1] != NULL))
        {
            const float* in_0 = Work_buffer_get_contents(in_wbs[0]);
            const float* in_1 = Work_buffer_get_contents(in_wbs[1]);
            const float gain = (float)freeverb->gain;
            for (int32_t i = 0; i < frame_count; ++i)
                comb_input[i] = (in_0[i] + in_1[i]) * gain;
        }
        else if ((in_wbs[0] == NULL) != (in_wbs[1] == NULL))
        {
            // Use the valid channel as the input for both channels
            const Work_buffer* valid_in_wb = (in_wbs[0] != NULL) ? in_wbs[0] : in_wbs[1];
            const float* valid_in = Work_buffer_get_contents(valid_in_wb);
            const float gain = (float)(freeverb->gain * 2.0);
            for (int32_t i = 0; i < frame_count; ++i)
                comb_input[i] = valid_in[i] * gain;
        }
        else
        {
            Work_buffer_clear(comb_input_wb, 0, frame_count);
        }

#if KQT_SSE
        const unsigned int old_ftoz = _MM_GET_FLUSH_ZERO_MODE();
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif

        for (int ch = 0; ch < 2; ++ch)
        {
            Work_buffer* out_wb = out_wbs[ch];
            if (out_wb == NULL)
                continue;

            Work_buffer_clear(out_wb, 0, frame_count);
            float* out_contents = Work_buffer_get_contents_mut(out_wb);

            for (int comb_index = 0; comb_index < FREEVERB_COMBS; ++comb_index)
                Freeverb_comb_process(
                        fstate->combs[ch][comb_index],
                        out_contents,
                        comb_input,
                        refls,
                        damps,
                        frame_count);

            for (int allpass_index = 0; allpass_index < FREEVERB_ALLPASSES; ++allpass_index)
                Freeverb_allpass_process(
                        fstate->allpasses[ch][allpass_index], out_contents, frame_count);
        }

#if KQT_SSE
        _MM_SET_FLUSH_ZERO_MODE(old_ftoz);
#endif
    }

    return;
}


Device_state* new_Freeverb_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Freeverb_pstate* fpstate = memory_alloc_item(Freeverb_pstate);
    if (fpstate == NULL)
        return NULL;

    if (!Proc_state_init(&fpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(fpstate);
        return NULL;
    }

    fpstate->parent.destroy = del_Freeverb_pstate;
    fpstate->parent.set_audio_rate = Freeverb_pstate_set_audio_rate;
    fpstate->parent.reset = Freeverb_pstate_reset;
    fpstate->parent.render_mixed = Freeverb_pstate_render_mixed;
    fpstate->parent.clear_history = Freeverb_pstate_clear_history;

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < FREEVERB_COMBS; ++i)
            fpstate->combs[ch][i] = NULL;

        for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
            fpstate->allpasses[ch][i] = NULL;
    }

    for (int i = 0; i < FREEVERB_COMBS; ++i)
    {
        const int32_t left_size = (int32_t)max(1, comb_tuning[i] * audio_rate);

        fpstate->combs[0][i] = new_Freeverb_comb(left_size);
        if (fpstate->combs[0][i] == NULL)
        {
            del_Device_state(&fpstate->parent.parent);
            return NULL;
        }

        const int32_t right_size =
            (int32_t)max(1, (comb_tuning[i] + stereo_spread) * audio_rate);

        fpstate->combs[1][i] = new_Freeverb_comb(right_size);
        if (fpstate->combs[1][i] == NULL)
        {
            del_Device_state(&fpstate->parent.parent);
            return NULL;
        }
    }

    for (int i = 0; i < FREEVERB_ALLPASSES; ++i)
    {
        const int32_t left_size = (int32_t)max(1, allpass_tuning[i] * audio_rate);

        if (fpstate->allpasses[0][i] == NULL)
        {
            fpstate->allpasses[0][i] = new_Freeverb_allpass(left_size);
            if (fpstate->allpasses[0][i] == NULL)
            {
                del_Device_state(&fpstate->parent.parent);
                return NULL;
            }
        }

        const int32_t right_size =
            (int32_t)max(1, (allpass_tuning[i] + stereo_spread) * audio_rate);

        if (fpstate->allpasses[1][i] == NULL)
        {
            fpstate->allpasses[1][i] = new_Freeverb_allpass(right_size);
            if (fpstate->allpasses[1][i] == NULL)
            {
                del_Device_state(&fpstate->parent.parent);
                return NULL;
            }
        }
    }

    return &fpstate->parent.parent;
}


