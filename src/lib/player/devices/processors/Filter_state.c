

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
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
#include <memory.h>
#include <player/devices/Filter.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/devices/Voice_state.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#ifndef FILTER_ORDER
#define FILTER_ORDER 2
#endif


typedef struct Single_filter_state
{
    double coeffs[FILTER_ORDER];
    double mul;
    double history1[KQT_BUFFERS_MAX][FILTER_ORDER];
    double history2[KQT_BUFFERS_MAX][FILTER_ORDER];
} Single_filter_state;


typedef struct Filter_state_impl
{
    bool anything_rendered;

    double applied_lowpass;
    double applied_resonance;
    double true_lowpass;
    double true_resonance;
    double lowpass_xfade_pos;
    double lowpass_xfade_update;
    int lowpass_xfade_state_used;
    int lowpass_state_used;
    Single_filter_state lowpass_state[2];

    Linear_controls cutoff;
    Linear_controls resonance;
} Filter_state_impl;


static void Filter_state_impl_init(Filter_state_impl* fimpl, const Proc_filter* filter)
{
    assert(fimpl != NULL);

    fimpl->anything_rendered = false;

    fimpl->applied_lowpass = FILTER_DEFAULT_CUTOFF;
    fimpl->applied_resonance = FILTER_DEFAULT_RESONANCE;
    fimpl->true_lowpass = INFINITY;
    fimpl->true_resonance = 0.5;
    fimpl->lowpass_state_used = -1;
    fimpl->lowpass_xfade_state_used = -1;
    fimpl->lowpass_xfade_pos = 1;
    fimpl->lowpass_xfade_update = 0;

    for (int i = 0; i < FILTER_ORDER; ++i)
    {
        fimpl->lowpass_state[0].coeffs[i] = 0;
        fimpl->lowpass_state[1].coeffs[i] = 0;

        for (int k = 0; k < KQT_BUFFERS_MAX; ++k)
        {
            fimpl->lowpass_state[0].history1[k][i] = 0;
            fimpl->lowpass_state[0].history2[k][i] = 0;
            fimpl->lowpass_state[1].history1[k][i] = 0;
            fimpl->lowpass_state[1].history2[k][i] = 0;
        }
    }

    Linear_controls_init(&fimpl->cutoff);
    Linear_controls_init(&fimpl->resonance);
    Linear_controls_set_value(&fimpl->cutoff, filter->cutoff);
    Linear_controls_set_value(&fimpl->resonance, filter->resonance);

    return;
}


static void Filter_state_impl_set_audio_rate(
        Filter_state_impl* fimpl, int32_t audio_rate)
{
    assert(fimpl != NULL);
    assert(audio_rate > 0);

    Linear_controls_set_audio_rate(&fimpl->cutoff, audio_rate);
    Linear_controls_set_audio_rate(&fimpl->resonance, audio_rate);

    return;
}


static void Filter_state_impl_set_tempo(
        Filter_state_impl* fimpl, double tempo)
{
    assert(fimpl != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Linear_controls_set_tempo(&fimpl->cutoff, tempo);
    Linear_controls_set_tempo(&fimpl->resonance, tempo);

    return;
}


static const int CONTROL_WB_CUTOFF = WORK_BUFFER_IMPL_1;
static const int CONTROL_WB_RESONANCE = WORK_BUFFER_IMPL_2;


static void Filter_state_impl_update_controls(
        Filter_state_impl* fimpl,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(fimpl != NULL);
    assert(wbs != NULL);

    const Work_buffer* cutoff_wb = Work_buffers_get_buffer(wbs, CONTROL_WB_CUTOFF);
    const Work_buffer* res_wb = Work_buffers_get_buffer(wbs, CONTROL_WB_RESONANCE);

    Linear_controls_fill_work_buffer(&fimpl->cutoff, cutoff_wb, buf_start, buf_stop);
    Linear_controls_fill_work_buffer(&fimpl->resonance, res_wb, buf_start, buf_stop);

    return;
}


static void Filter_state_impl_apply_filter_settings(
        Filter_state_impl* fimpl,
        Audio_buffer* in_buffer,
        Audio_buffer* out_buffer,
        double xfade_start,
        double xfade_step,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(fimpl != NULL);
    assert(in_buffer != NULL);
    assert(out_buffer != NULL);

    if ((fimpl->lowpass_state_used == -1) && (fimpl->lowpass_xfade_state_used == -1))
    {
        Audio_buffer_copy(out_buffer, in_buffer, buf_start, buf_stop);
        return;
    }

    assert(fimpl->lowpass_state_used != fimpl->lowpass_xfade_state_used);

    const float* in_bufs[] =
    {
        Audio_buffer_get_buffer(in_buffer, 0),
        Audio_buffer_get_buffer(in_buffer, 1),
    };

    float* out_bufs[] =
    {
        Audio_buffer_get_buffer(out_buffer, 0),
        Audio_buffer_get_buffer(out_buffer, 1),
    };

    // Get filter states used
    Single_filter_state* in_fst = (fimpl->lowpass_state_used > -1) ?
        &fimpl->lowpass_state[fimpl->lowpass_state_used] : NULL;
    Single_filter_state* out_fst = (fimpl->lowpass_xfade_state_used > -1) ?
        &fimpl->lowpass_state[fimpl->lowpass_xfade_state_used] : NULL;

    const double xfade_start_clamped = min(xfade_start, 1);

    for (int32_t ch = 0; ch < 2; ++ch)
    {
        double xfade = xfade_start_clamped;

        const float* in_buf = in_bufs[ch];
        float* out_buf = out_bufs[ch];

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float input = in_buf[i];
            double result = input;

            // Apply primary filter
            if (in_fst != NULL)
            {
                result = nq_zero_filter(
                        FILTER_ORDER, in_fst->history1[ch], input);
                result = iir_filter_strict_cascade_even_order(
                        FILTER_ORDER, in_fst->coeffs, in_fst->history2[ch], result);
                result *= in_fst->mul;
            }

            // Apply secondary filter with crossfade
            if (xfade < 1)
            {
                result *= xfade;

                double fade_result = input;

                if (out_fst != NULL)
                {
                    fade_result = nq_zero_filter(
                            FILTER_ORDER, out_fst->history1[ch], input);
                    fade_result = iir_filter_strict_cascade_even_order(
                            FILTER_ORDER,
                            out_fst->coeffs,
                            out_fst->history2[ch],
                            fade_result);
                    fade_result *= out_fst->mul;
                }

                result += fade_result * (1 - xfade);

                xfade += xfade_step;
            }

            out_buf[i] = result;
        }
    }

    return;
}


#define CUTOFF_INF_LIMIT 100.0
#define CUTOFF_BIAS 81.37631656229591


static double get_cutoff_freq(double param)
{
    assert(isfinite(param));

    if (param >= CUTOFF_INF_LIMIT)
        return INFINITY;

    param = max(-100, param);
    return exp2((param + CUTOFF_BIAS) / 12.0);
}


static double get_resonance(double param)
{
    const double clamped_res = clamp(param, 0, 100);
    const double resonance = pow(1.055, clamped_res) * 0.5;
    return resonance;
}


#define FILTER_XFADE_SPEED_MIN 40.0
#define FILTER_XFADE_SPEED_MAX 200.0


static double get_xfade_step(double audio_rate, double true_lowpass, double resonance)
{
    assert(audio_rate > 0);

    if (true_lowpass >= audio_rate * 0.5)
        return FILTER_XFADE_SPEED_MAX / audio_rate;

    static const double xfade_range = FILTER_XFADE_SPEED_MAX - FILTER_XFADE_SPEED_MIN;

    const double clamped_res = clamp(resonance, 0, 100);
    const double xfade_norm = clamped_res / 100;
    const double xfade_speed = FILTER_XFADE_SPEED_MAX - xfade_norm * xfade_range;

    return xfade_speed / audio_rate;
}


static void Filter_state_impl_apply_work_buffers(
        Filter_state_impl* fimpl,
        const Work_buffers* wbs,
        Audio_buffer* in_buffer,
        Audio_buffer* out_buffer,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    const float* cutoffs = Work_buffers_get_buffer_contents(wbs, CONTROL_WB_CUTOFF);
    const float* resonances =
        Work_buffers_get_buffer_contents(wbs, CONTROL_WB_RESONANCE);

    static const double max_true_lowpass_change = 0.01;
    static const double max_resonance_change = 0.01;

    fimpl->lowpass_xfade_update = get_xfade_step(
            audio_rate, fimpl->true_lowpass, fimpl->applied_resonance);

    const double nyquist = (double)audio_rate * 0.5;

    int32_t apply_filter_start = buf_start;
    int32_t apply_filter_stop = buf_stop;
    double xfade_start = fimpl->lowpass_xfade_pos;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float cutoff = cutoffs[i];
        const float resonance = resonances[i];

        // TODO: apply force->filter envelope if applicable
        /*
        if (proc->au_params->env_force_filter_enabled &&
                fimpl->lowpass_xfade_pos >= 1)
        {
            double force = Cond_work_buffer_get_value(actual_forces, i);
            if (force > 1)
                force = 1;

            double factor = Envelope_get_value(proc->au_params->env_force_filter, force);
            assert(isfinite(factor));
            cutoff += (factor * 100) - 100;
        }
        // */

        // Initialise new filter settings if needed
        if (fimpl->lowpass_xfade_pos >= 1 &&
                ((fabs(cutoff - fimpl->applied_lowpass) >
                    max_true_lowpass_change) ||
                 (fabs(resonance - fimpl->applied_resonance) >
                    max_resonance_change)))
        {
            // Apply previous filter settings to the signal
            apply_filter_stop = i;
            Filter_state_impl_apply_filter_settings(
                    fimpl,
                    in_buffer,
                    out_buffer,
                    xfade_start,
                    fimpl->lowpass_xfade_update,
                    apply_filter_start,
                    apply_filter_stop);

            // Set up new range for next filter processing
            apply_filter_start = i;
            apply_filter_stop = buf_stop;

            fimpl->lowpass_xfade_state_used = fimpl->lowpass_state_used;

            // Only apply crossfade if we have rendered audio
            if (fimpl->anything_rendered)
                fimpl->lowpass_xfade_pos = 0;
            else
                fimpl->lowpass_xfade_pos = 1;

            fimpl->applied_lowpass = cutoff;
            fimpl->true_lowpass = get_cutoff_freq(fimpl->applied_lowpass);

            fimpl->applied_resonance = resonance;
            fimpl->true_resonance = get_resonance(fimpl->applied_resonance);

            fimpl->lowpass_xfade_update = get_xfade_step(
                    audio_rate, fimpl->true_lowpass, fimpl->applied_resonance);

            if (fimpl->true_lowpass < nyquist)
            {
                const int new_state = 1 - abs(fimpl->lowpass_state_used);
                const double lowpass = max(fimpl->true_lowpass, 1);
                two_pole_filter_create(lowpass / audio_rate,
                        fimpl->true_resonance,
                        0,
                        fimpl->lowpass_state[new_state].coeffs,
                        &fimpl->lowpass_state[new_state].mul);
                for (int a = 0; a < KQT_BUFFERS_MAX; ++a)
                {
                    for (int k = 0; k < FILTER_ORDER; ++k)
                    {
                        fimpl->lowpass_state[new_state].history1[a][k] = 0;
                        fimpl->lowpass_state[new_state].history2[a][k] = 0;
                    }
                }
                fimpl->lowpass_state_used = new_state;
                //fprintf(stderr, "created filter with cutoff %f\n", cutoff);
            }
            else
            {
                if (fimpl->lowpass_state_used == -1)
                    fimpl->lowpass_xfade_pos = 1;

                fimpl->lowpass_state_used = -1;
            }

            xfade_start = fimpl->lowpass_xfade_pos;
        }

        fimpl->anything_rendered = true;

        fimpl->lowpass_xfade_pos += fimpl->lowpass_xfade_update;
    }

    // Apply previous filter settings to the remaining signal
    Filter_state_impl_apply_filter_settings(
            fimpl,
            in_buffer,
            out_buffer,
            xfade_start,
            fimpl->lowpass_xfade_update,
            apply_filter_start,
            apply_filter_stop);

    return;
}


typedef struct Filter_pstate
{
    Proc_state parent;

    Filter_state_impl state_impl;
} Filter_pstate;


static bool Filter_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    Filter_state_impl_set_audio_rate(&fpstate->state_impl, audio_rate);
    Filter_state_impl_init(&fpstate->state_impl, filter);

    return true;
}


static void Filter_pstate_set_tempo(Device_state* dstate, double tempo)
{
    assert(dstate != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    Filter_state_impl_set_tempo(&fpstate->state_impl, tempo);

    return;
}


static void Filter_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    Filter_state_impl_init(&fpstate->state_impl, filter);

    return;
}


static void Filter_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Filter_pstate* fpstate = (Filter_pstate*)dstate;

    Filter_state_impl_set_tempo(&fpstate->state_impl, tempo);
    Filter_state_impl_update_controls(&fpstate->state_impl, wbs, buf_start, buf_stop);

    // Get input
    Audio_buffer* in_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (in_buffer == NULL)
        return;

    // Get output
    Audio_buffer* out_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    Filter_state_impl_apply_work_buffers(
            &fpstate->state_impl,
            wbs,
            in_buffer,
            out_buffer,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    return;
}


bool Filter_pstate_set_cutoff(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    Linear_controls_set_value(&fpstate->state_impl.cutoff, value);

    return true;
}


bool Filter_pstate_set_resonance(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    Linear_controls_set_value(&fpstate->state_impl.resonance, value);

    return true;
}


Device_state* new_Filter_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Filter_pstate* fpstate = memory_alloc_item(Filter_pstate);
    if ((fpstate == NULL) ||
            !Proc_state_init(&fpstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(fpstate);
        return NULL;
    }

    fpstate->parent.set_audio_rate = Filter_pstate_set_audio_rate;
    fpstate->parent.set_tempo = Filter_pstate_set_tempo;
    fpstate->parent.reset = Filter_pstate_reset;
    fpstate->parent.render_mixed = Filter_pstate_render_mixed;

    const Proc_filter* filter = (const Proc_filter*)device->dimpl;
    Filter_state_impl_init(&fpstate->state_impl, filter);
    Filter_state_impl_set_audio_rate(&fpstate->state_impl, audio_rate);

    return (Device_state*)fpstate;
}


typedef struct Filter_vstate
{
    Voice_state parent;

    Filter_state_impl state_impl;
} Filter_vstate;


size_t Filter_vstate_get_size(void)
{
    return sizeof(Filter_vstate);
}


int32_t Filter_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Filter_vstate* fvstate = (Filter_vstate*)vstate;

    Filter_state_impl_set_tempo(&fvstate->state_impl, tempo);
    Filter_state_impl_update_controls(&fvstate->state_impl, wbs, buf_start, buf_stop);

    // Get input
    Audio_buffer* in_buffer =
        Proc_state_get_voice_buffer_mut(proc_state, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (in_buffer == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    Audio_buffer* out_buffer =
        Proc_state_get_voice_buffer_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    const Device_state* dstate = (const Device_state*)proc_state;
    Filter_state_impl_apply_work_buffers(
            &fvstate->state_impl,
            wbs,
            in_buffer,
            out_buffer,
            buf_start,
            buf_stop,
            dstate->audio_rate);

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Filter_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Filter_vstate_render_voice;

    Filter_vstate* fvstate = (Filter_vstate*)vstate;

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;
    Filter_state_impl_init(&fvstate->state_impl, filter);
    Filter_state_impl_set_audio_rate(&fvstate->state_impl, dstate->audio_rate);

    return;
}


