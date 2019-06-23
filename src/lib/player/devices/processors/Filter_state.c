

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2018-2019
 *          Sami Koistinen, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Filter_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_filter.h>
#include <intrinsics.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/fast_exp2.h>
#include <mathnum/fast_sin.h>
#include <mathnum/fast_tan.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#define MIN_CUTOFF_RATIO 0.00003
#define MAX_CUTOFF_RATIO 0.49


#define ENABLE_FILTER_SSE (KQT_SSE && KQT_SSE2 && KQT_SSE4_1)


typedef struct Filter_ch_state
{
    float s1;
    float s2;
} Filter_ch_state;


typedef struct Filter_state_impl
{
    Filter_ch_state states[2];
} Filter_state_impl;


static void Filter_state_impl_init(Filter_state_impl* fimpl, const Proc_filter* filter)
{
    rassert(fimpl != NULL);
    rassert(filter != NULL);

    for (int ch = 0; ch < 2; ++ch)
    {
        fimpl->states[ch].s1 = 0;
        fimpl->states[ch].s2 = 0;
    }

    return;
}


static bool Filter_state_is_neutral(const Filter_state_impl* fimpl)
{
    rassert(fimpl != NULL);

    for (int ch = 0; ch < 2; ++ch)
    {
        if ((fimpl->states[ch].s1 != 0) || (fimpl->states[ch].s2 != 0))
            return false;
    }

    return true;
}


static float get_cutoff(double rel_freq)
{
    rassert(rel_freq > 0);
    rassert(rel_freq < 0.5);
    return (float)tan(PI * rel_freq);
}


static float get_cutoff_ratio(double cutoff_param, int32_t audio_rate)
{
    const double clamped_cutoff_param = clamp(cutoff_param, -36, 136);
    const double cutoff_ratio =
        cents_to_Hz((clamped_cutoff_param - 24) * 100) / audio_rate;
    return (float)clamp(cutoff_ratio, MIN_CUTOFF_RATIO, MAX_CUTOFF_RATIO);
}


#if ENABLE_FILTER_SSE

static __m128 get_cutoff_fast_f4(__m128 rel_freq)
{
    const __m128 pi = _mm_set1_ps((float)PI);
    const __m128 scaled_freq = _mm_mul_ps(pi, rel_freq);
    return fast_tan_pos_f4(scaled_freq);
}

#else

static float get_cutoff_fast(double rel_freq)
{
    dassert(rel_freq > 0);
    dassert(rel_freq < 0.5);

    const double scaled_freq = PI * rel_freq;

    return (float)fast_tan(scaled_freq);
}

#endif


static const int CONTROL_WB_CUTOFF = WORK_BUFFER_IMPL_1;
static const int CONTROL_WB_RESONANCE = WORK_BUFFER_IMPL_2;
static const int FILTER_WB_SILENT_INPUT = WORK_BUFFER_IMPL_3;


static void Filter_state_impl_apply_input_buffers(
        Filter_state_impl* fimpl,
        const Proc_filter* filter,
        const Work_buffer* cutoff_wb,
        const Work_buffer* resonance_wb,
        const Work_buffers* wbs,
        Work_buffer* in_wbs[2],
        Work_buffer* out_wbs[2],
        int32_t frame_count,
        int32_t audio_rate)
{
    rassert(fimpl != NULL);
    rassert(wbs != NULL);
    rassert(in_wbs != NULL);
    rassert(out_wbs != NULL);
    rassert(audio_rate > 0);

    float* cutoffs = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_CUTOFF);

    int32_t params_const_start = 0;

    // Fill cutoff buffer
    {
        int32_t fast_cutoff_stop = 0;
        float const_cutoff = NAN;

        if (Work_buffer_is_valid(cutoff_wb))
        {
            const int32_t const_start = Work_buffer_get_const_start(cutoff_wb);
            fast_cutoff_stop = min(const_start, frame_count);
            const float* cutoff_buf = Work_buffer_get_contents(cutoff_wb);

            // Get cutoff values from input
#if ENABLE_FILTER_SSE
            const __m128 inv_audio_rate = _mm_set_ps1((float)(1.0 / audio_rate));
            for (int32_t i = 0; i < fast_cutoff_stop; i += 4)
            {
                const __m128 cutoff_param = _mm_load_ps(cutoff_buf + i);
                const __m128 offset = _mm_set_ps1(-24);
                const __m128 scale = _mm_set_ps1(100);
                const __m128 cutoff_ratio = _mm_mul_ps(
                        fast_cents_to_Hz_f4(
                            _mm_mul_ps(_mm_add_ps(cutoff_param, offset), scale)),
                        inv_audio_rate);

                const __m128 min_ratio = _mm_set_ps1((float)MIN_CUTOFF_RATIO);
                const __m128 max_ratio = _mm_set_ps1((float)MAX_CUTOFF_RATIO);
                const __m128 cutoff_ratio_clamped =
                    _mm_min_ps(_mm_max_ps(min_ratio, cutoff_ratio), max_ratio);

                const __m128 cutoff = get_cutoff_fast_f4(cutoff_ratio_clamped);
                _mm_store_ps(cutoffs + i, cutoff);
            }
#else
            for (int32_t i = 0; i < fast_cutoff_stop; ++i)
            {
                const double cutoff_param = cutoff_buf[i];
                const double cutoff_ratio =
                    fast_cents_to_Hz((cutoff_param - 24) * 100) / audio_rate;
                const double cutoff_ratio_clamped =
                    clamp(cutoff_ratio, MIN_CUTOFF_RATIO, MAX_CUTOFF_RATIO);

                cutoffs[i] = get_cutoff_fast(cutoff_ratio_clamped);
            }
#endif

            if (fast_cutoff_stop < frame_count)
            {
                const double cutoff_ratio =
                    get_cutoff_ratio(cutoff_buf[fast_cutoff_stop], audio_rate);
                const_cutoff = get_cutoff(cutoff_ratio);
            }
        }
        else
        {
            const double cutoff_ratio = get_cutoff_ratio(filter->cutoff, audio_rate);
            const_cutoff = get_cutoff(cutoff_ratio);
        }

        for (int32_t i = fast_cutoff_stop; i < frame_count; ++i)
            cutoffs[i] = const_cutoff;

        params_const_start = max(params_const_start, fast_cutoff_stop);
    }

    float* resonances = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_RESONANCE);

    // Fill resonance buffer
    {
        // We are going to warp the normalised resonance with: (bias^res - 1) / (bias - 1)
        const double res_bias_base = 50.0;
        const double res_bias_base_log2 = log2(res_bias_base);

        int32_t fast_res_stop = 0;
        float const_res = NAN;

        if (Work_buffer_is_valid(resonance_wb))
        {
            const int32_t const_start = Work_buffer_get_const_start(resonance_wb);
            fast_res_stop = min(const_start, frame_count);
            const float* resonance_buf = Work_buffer_get_contents(resonance_wb);

            // Get resonance values from input
            for (int32_t i = 0; i < fast_res_stop; ++i)
            {
                const double res_param = resonance_buf[i];
                const double biased_res_exp =
                    fast_exp2(res_bias_base_log2 * (100 - res_param) / 100.0);
                const float biased_res =
                    (float)((biased_res_exp - 1) * 2.0 / (res_bias_base - 1));

                resonances[i] = biased_res;
            }

            if (fast_res_stop < frame_count)
            {
                const double biased_res_exp = exp2(
                        res_bias_base_log2 *
                        (100 - resonance_buf[fast_res_stop]) / 100.0);
                const_res = (float)((biased_res_exp - 1) * 2.0 / (res_bias_base - 1));
            }
        }
        else
        {
            // Get our default resonance
            const double biased_res_exp =
                exp2(res_bias_base_log2 * (100 - filter->resonance) / 100.0);
            const_res = (float)((biased_res_exp - 1) * 2.0 / (res_bias_base - 1));
        }

        for (int32_t i = fast_res_stop; i < frame_count; ++i)
            resonances[i] = const_res;

        params_const_start = max(params_const_start, fast_res_stop);
    }

    bool empty_input_created = false;

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* in_wb = in_wbs[ch];

        Work_buffer* out_wb = out_wbs[ch];
        if (out_wb == NULL)
            continue;

        if (in_wb == NULL)
        {
            // If we no longer get valid input, we still need to produce a
            // valid output signal as the filter takes a while to adapt
            in_wb = Work_buffers_get_buffer_mut(wbs, FILTER_WB_SILENT_INPUT);
            if (!empty_input_created)
            {
                Work_buffer_clear(in_wb, 0, frame_count);
                empty_input_created = true;
            }
        }

        // Apply the filter
        const float* in = Work_buffer_get_contents(in_wb);
        float* out = Work_buffer_get_contents_mut(out_wb);

        Filter_ch_state* fstate = &fimpl->states[ch];

        float s1 = fstate->s1;
        float s2 = fstate->s2;

        const bool is_lowpass = (filter->type == FILTER_TYPE_LOWPASS);

        for (int32_t i = 0; i < frame_count; ++i)
        {
            const float x = *in++;
            const float g = cutoffs[i];
            const float k = resonances[i];

            const float hp_mult = 1.0f / (1.0f + (k * g) + (g * g));
            const float hp_sample = (x - s1 * (k + g) - s2) * hp_mult;

            const float input_1 = g * hp_sample;
            const float bp_sample = input_1 + s1;
            s1 = input_1 + bp_sample;

            const float input_2 = g * bp_sample;
            const float lp_sample = input_2 + s2;
            s2 = input_2 + lp_sample;

            if (is_lowpass)
                *out++ = lp_sample;
            else
                *out++ = hp_sample;
        }

        fstate->s1 = s1;
        fstate->s2 = s2;
    }

    return;
}


typedef struct Filter_pstate
{
    Proc_state parent;

    Filter_state_impl state_impl;
} Filter_pstate;


static void Filter_pstate_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    Filter_state_impl_init(&fpstate->state_impl, filter);

    return;
}


enum
{
    PORT_IN_AUDIO_L = 0,
    PORT_IN_AUDIO_R,
    PORT_IN_CUTOFF,
    PORT_IN_RESONANCE,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static void Filter_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Filter_pstate* fpstate = (Filter_pstate*)dstate;

    // Get parameter inputs
    const Work_buffer* cutoff_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    const Work_buffer* resonance_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_RESONANCE);

    // Get audio inputs
    Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        in_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);
        if (!Work_buffer_is_valid(in_wbs[ch]))
            in_wbs[ch] = NULL;
    }

    // Get audio outputs
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);
    if ((out_wbs[0] == NULL) && (out_wbs[1] == NULL))
        return;

    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_state_impl_apply_input_buffers(
            &fpstate->state_impl,
            filter,
            cutoff_wb,
            resonance_wb,
            wbs,
            in_wbs,
            out_wbs,
            frame_count,
            dstate->audio_rate);

    return;
}


Device_state* new_Filter_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Filter_pstate* fpstate = memory_alloc_item(Filter_pstate);
    if ((fpstate == NULL) ||
            !Proc_state_init(&fpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(fpstate);
        return NULL;
    }

    fpstate->parent.reset = Filter_pstate_reset;
    fpstate->parent.render_mixed = Filter_pstate_render_mixed;

    const Proc_filter* filter = (const Proc_filter*)device->dimpl;
    Filter_state_impl_init(&fpstate->state_impl, filter);

    return (Device_state*)fpstate;
}


typedef struct Filter_vstate
{
    Voice_state parent;

    Filter_state_impl state_impl;
} Filter_vstate;


int32_t Filter_vstate_get_size(void)
{
    return sizeof(Filter_vstate);
}


int32_t Filter_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Filter_vstate* fvstate = (Filter_vstate*)vstate;

    // Get parameter inputs
    const Work_buffer* cutoff_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    const Work_buffer* resonance_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_RESONANCE);

    // Get audio inputs
    Work_buffer* in_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
    {
        in_wbs[ch] = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L + ch);
        if (!Work_buffer_is_valid(in_wbs[ch]))
            in_wbs[ch] = NULL;
    }

    // Get audio outputs
    Work_buffer* out_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        out_wbs[ch] = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L + ch);
    if ((out_wbs[0] == NULL) && (out_wbs[1] == NULL))
    {
        vstate->active = false;
        return 0;
    }

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_state_impl_apply_input_buffers(
            &fvstate->state_impl,
            filter,
            cutoff_wb,
            resonance_wb,
            wbs,
            in_wbs,
            out_wbs,
            frame_count,
            dstate->audio_rate);

    if ((in_wbs[0] == NULL) &&
            (in_wbs[1] == NULL) &&
            Filter_state_is_neutral(&fvstate->state_impl))
        vstate->active = false;

    return frame_count;
}


void Filter_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Filter_vstate* fvstate = (Filter_vstate*)vstate;

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;
    Filter_state_impl_init(&fvstate->state_impl, filter);

    return;
}


