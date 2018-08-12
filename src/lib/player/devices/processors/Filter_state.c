

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
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
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/fast_exp2.h>
#include <mathnum/fast_sin.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Filter_ch_state
{
    double s1;
    double s2;
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


static float get_cutoff(double rel_freq)
{
    rassert(rel_freq > 0);
    rassert(rel_freq < 0.25);
    return (float)tan(2.0 * PI * rel_freq);
}


static float get_cutoff_fast(double rel_freq)
{
    dassert(rel_freq > 0);
    dassert(rel_freq < 0.25);

    const double scaled_freq = 2.0 * PI * rel_freq;

    return (float)(fast_sin(scaled_freq) / fast_sin((PI * 0.5) + scaled_freq));
}


static float get_cutoff_ratio(double cutoff_param, int32_t audio_rate)
{
    const double cutoff_ratio = cents_to_Hz((cutoff_param - 24) * 100) / audio_rate;
    return (float)clamp(cutoff_ratio, 0.0001, 0.2499);
}


static const int CONTROL_WB_CUTOFF = WORK_BUFFER_IMPL_1;
static const int CONTROL_WB_RESONANCE = WORK_BUFFER_IMPL_2;
static const int FILTER_WB_HP_MULT = WORK_BUFFER_IMPL_3;


static void Filter_state_impl_apply_input_buffers(
        Filter_state_impl* fimpl,
        const Proc_filter* filter,
        const Work_buffer* cutoff_wb,
        const Work_buffer* resonance_wb,
        const Work_buffers* wbs,
        Work_buffer* in_buffers[2],
        Work_buffer* out_buffers[2],
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    rassert(fimpl != NULL);
    rassert(wbs != NULL);
    rassert(in_buffers != NULL);
    rassert(out_buffers != NULL);
    rassert(audio_rate > 0);

    float* cutoffs = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_CUTOFF);

    int32_t params_const_start = buf_start;

    // Fill cutoff buffer
    {
        int32_t fast_cutoff_stop = buf_start;
        float const_cutoff = NAN;

        if (cutoff_wb != NULL)
        {
            const int32_t const_start = Work_buffer_get_const_start(cutoff_wb);
            fast_cutoff_stop = clamp(const_start, buf_start, buf_stop);
            const float* cutoff_buf = Work_buffer_get_contents(cutoff_wb);

            // Get cutoff values from input
            for (int32_t i = buf_start; i < fast_cutoff_stop; ++i)
            {
                const double cutoff_param = cutoff_buf[i];
                const double cutoff_ratio =
                    fast_cents_to_Hz((cutoff_param - 24) * 100) / audio_rate;
                const double cutoff_ratio_clamped = clamp(cutoff_ratio, 0.0001, 0.2499);

                cutoffs[i] = get_cutoff_fast(cutoff_ratio_clamped);
            }

            if (fast_cutoff_stop < buf_stop)
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

        for (int32_t i = fast_cutoff_stop; i < buf_stop; ++i)
            cutoffs[i] = const_cutoff;

        params_const_start = max(params_const_start, fast_cutoff_stop);
    }

    float* resonances = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_RESONANCE);

    // Fill resonance buffer
    {
        // We are going to warp the normalised resonance with: (bias^res - 1) / (bias - 1)
        const double res_bias_base = 50.0;
        const double res_bias_base_log2 = log2(res_bias_base);

        int32_t fast_res_stop = buf_start;
        float const_res = NAN;

        if (resonance_wb != NULL)
        {
            const int32_t const_start = Work_buffer_get_const_start(resonance_wb);
            fast_res_stop = clamp(const_start, buf_start, buf_stop);
            const float* resonance_buf = Work_buffer_get_contents(resonance_wb);

            // Get resonance values from input
            for (int32_t i = buf_start; i < fast_res_stop; ++i)
            {
                const double res_param = resonance_buf[i];
                const double biased_res_exp =
                    fast_exp2(res_bias_base_log2 * (100 - res_param) / 100.0);
                const float biased_res =
                    (float)((biased_res_exp - 1) * 2.0 / (res_bias_base - 1));

                resonances[i] = biased_res;
            }

            if (fast_res_stop < buf_stop)
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

        for (int32_t i = fast_res_stop; i < buf_stop; ++i)
            resonances[i] = const_res;

        params_const_start = max(params_const_start, fast_res_stop);
    }

    // Apply the filter
    float* hp_mults = Work_buffers_get_buffer_contents_mut(wbs, FILTER_WB_HP_MULT);
    for (int32_t i = buf_start; i < params_const_start; ++i)
    {
        const double g = 0.5 * cutoffs[i];
        const double k = resonances[i];

        hp_mults[i] = (float)(1.0 / (1.0 + k * g + g * g));
    }
    if (params_const_start < buf_stop)
    {
        const double g = 0.5 * cutoffs[params_const_start];
        const double k = resonances[params_const_start];
        const float mult = (float)(1.0 / (1.0 + k * g + g * g));

        for (int32_t i = params_const_start; i < buf_stop; ++i)
            hp_mults[i] = mult;
    }

    for (int ch = 0; ch < 2; ++ch)
    {
        if ((in_buffers[ch] == NULL) || (out_buffers[ch] == NULL))
            continue;

        Filter_ch_state* state = &fimpl->states[ch];

        const float* in_buf = Work_buffer_get_contents(in_buffers[ch]);
        float* out_buf = Work_buffer_get_contents_mut(out_buffers[ch]);

        double s1 = state->s1;
        double s2 = state->s2;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const double x = in_buf[i];
            const double g = 0.5 * cutoffs[i];
            const double k = resonances[i];

            const double hp_sample = (x - s1 * (k + g) - s2) * hp_mults[i];

            const double input1 = g * hp_sample;
            const double bp_sample = input1 + s1;
            s1 = input1 + bp_sample;

            const double input2 = g * bp_sample;
            const double lp_sample = input2 + s2;
            s2 = input2 + lp_sample;

            if (filter->type == FILTER_TYPE_LOWPASS)
                out_buf[i] = (float)lp_sample;
            else
                out_buf[i] = (float)hp_sample;
        }

        state->s1 = s1;
        state->s2 = s2;
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
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(wbs != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Filter_pstate* fpstate = (Filter_pstate*)dstate;

    // Get parameter inputs
    const Work_buffer* cutoff_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    const Work_buffer* resonance_wb = Device_thread_state_get_mixed_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_RESONANCE);

    // Get audio buffers
    Work_buffer* in_buffers[2] =
    {
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L),
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_R),
    };

    // Get output
    Work_buffer* out_buffers[2] =
    {
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L),
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_R),
    };

    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_state_impl_apply_input_buffers(
            &fpstate->state_impl,
            filter,
            cutoff_wb,
            resonance_wb,
            wbs,
            in_buffers,
            out_buffers,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    return;
}


bool Filter_pstate_set_type(
        Device_state* dstate, const Key_indices indices, int64_t type)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);
    ignore(type);

    /*
    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    fpstate->state_impl.type = filter->type;
    */

    return true;
}


bool Filter_pstate_set_cutoff(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    /*
    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    fpstate->state_impl.def_cutoff = value;
    */

    return true;
}


bool Filter_pstate_set_resonance(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    /*
    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    fpstate->state_impl.def_resonance = value;
    */

    return true;
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
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Filter_vstate* fvstate = (Filter_vstate*)vstate;

    // Get parameter inputs
    const Work_buffer* cutoff_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    const Work_buffer* resonance_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_RESONANCE);

    // Get input
    Work_buffer* in_buffers[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_L),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_AUDIO_R),
    };
    if ((in_buffers[0] == NULL) && (in_buffers[1] == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    Work_buffer* out_buffers[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_L),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO_R),
    };

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_state_impl_apply_input_buffers(
            &fvstate->state_impl,
            filter,
            cutoff_wb,
            resonance_wb,
            wbs,
            in_buffers,
            out_buffers,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    return buf_stop;
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


