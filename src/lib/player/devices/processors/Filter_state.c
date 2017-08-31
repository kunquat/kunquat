

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
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
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Filter.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#define FILTER_ORDER 2


typedef struct Single_filter_state
{
    Filter_type type;
    double coeffs[FILTER_ORDER];
    double mul;
    double history1[KQT_BUFFERS_MAX][FILTER_ORDER];
    double history2[KQT_BUFFERS_MAX][FILTER_ORDER];
} Single_filter_state;


typedef struct Filter_state_impl
{
    bool anything_rendered;

    Filter_type applied_type;
    double applied_cutoff;
    double applied_resonance;
    double true_cutoff;
    double true_resonance;
    double filter_xfade_pos;
    double filter_xfade_update;
    int filter_xfade_state_used;
    int filter_state_used;
    Single_filter_state sf_state[2];

    Filter_type type;
    double def_cutoff;
    double def_resonance;
} Filter_state_impl;


static void Filter_state_impl_init(Filter_state_impl* fimpl, const Proc_filter* filter)
{
    rassert(fimpl != NULL);

    fimpl->anything_rendered = false;

    fimpl->applied_type = filter->type;
    fimpl->applied_cutoff = FILTER_DEFAULT_CUTOFF;
    fimpl->applied_resonance = FILTER_DEFAULT_RESONANCE;
    fimpl->true_cutoff = INFINITY;
    fimpl->true_resonance = 0.5;
    fimpl->filter_state_used = -1;
    fimpl->filter_xfade_state_used = -1;
    fimpl->filter_xfade_pos = 1;
    fimpl->filter_xfade_update = 0;

    // Hack: Make sure that interesting highpass cutoff values are applied
    if (filter->type == FILTER_TYPE_HIGHPASS)
        fimpl->applied_cutoff = -100.0;

    for (int si = 0; si < 2; ++si)
    {
        fimpl->sf_state[si].type = filter->type;
        fimpl->sf_state[si].mul = 0;

        for (int i = 0; i < FILTER_ORDER; ++i)
        {
            fimpl->sf_state[si].coeffs[i] = 0;

            for (int ch = 0; ch < KQT_BUFFERS_MAX; ++ch)
            {
                fimpl->sf_state[si].history1[ch][i] = 0;
                fimpl->sf_state[si].history2[ch][i] = 0;
            }
        }
    }

    fimpl->type = filter->type;
    fimpl->def_cutoff = filter->cutoff;
    fimpl->def_resonance = filter->resonance;

    return;
}


static const int CONTROL_WB_CUTOFF = WORK_BUFFER_IMPL_1;
static const int CONTROL_WB_RESONANCE = WORK_BUFFER_IMPL_2;


static void Filter_state_impl_apply_filter_settings(
        Filter_state_impl* fimpl,
        Work_buffer* in_buffers[2],
        Work_buffer* out_buffers[2],
        double xfade_start,
        double xfade_step,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(fimpl != NULL);
    rassert(in_buffers != NULL);
    rassert(out_buffers != NULL);

    if ((fimpl->filter_state_used == -1) && (fimpl->filter_xfade_state_used == -1))
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            if ((in_buffers[ch] != NULL) && (out_buffers[ch] != NULL))
                Work_buffer_copy(out_buffers[ch], in_buffers[ch], buf_start, buf_stop);
        }
        return;
    }

    rassert(fimpl->filter_state_used != fimpl->filter_xfade_state_used);

    // Get filter states used
    Single_filter_state* in_fst = (fimpl->filter_state_used > -1) ?
        &fimpl->sf_state[fimpl->filter_state_used] : NULL;
    Single_filter_state* out_fst = (fimpl->filter_xfade_state_used > -1) ?
        &fimpl->sf_state[fimpl->filter_xfade_state_used] : NULL;

    const double xfade_start_clamped = min(xfade_start, 1);

    for (int32_t ch = 0; ch < 2; ++ch)
    {
        double xfade = xfade_start_clamped;

        if ((in_buffers[ch] == NULL) || (out_buffers[ch] == NULL))
            continue;

        const float* in_buf = Work_buffer_get_contents(in_buffers[ch]);
        float* out_buf = Work_buffer_get_contents_mut(out_buffers[ch]);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float input = in_buf[i];
            double result = input;

            // Apply primary filter
            if (in_fst != NULL)
            {
                if (in_fst->type == FILTER_TYPE_LOWPASS)
                    result = nq_zero_filter(
                            FILTER_ORDER, in_fst->history1[ch], input);
                else
                    result = dc_zero_filter(
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
                    if (out_fst->type == FILTER_TYPE_LOWPASS)
                        fade_result = nq_zero_filter(
                                FILTER_ORDER, out_fst->history1[ch], input);
                    else
                        fade_result = dc_zero_filter(
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

            out_buf[i] = (float)result;
        }
    }

    return;
}


#define CUTOFF_INF_LIMIT 100.0
#define CUTOFF_BIAS 81.37631656229591


static double get_cutoff_freq(double param)
{
    rassert(!isnan(param));

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


static double get_xfade_step(double audio_rate, double true_cutoff, double resonance)
{
    rassert(audio_rate > 0);

    if (true_cutoff >= audio_rate * 0.5)
        return FILTER_XFADE_SPEED_MAX / audio_rate;

    const double xfade_range = FILTER_XFADE_SPEED_MAX - FILTER_XFADE_SPEED_MIN;

    const double clamped_res = clamp(resonance, 0, 100);
    const double xfade_norm = clamped_res / 100;
    const double xfade_speed = FILTER_XFADE_SPEED_MAX - xfade_norm * xfade_range;

    return xfade_speed / audio_rate;
}


static void Filter_state_impl_apply_input_buffers(
        Filter_state_impl* fimpl,
        const float* cutoff_buf,
        const float* resonance_buf,
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

    if (cutoff_buf != NULL)
    {
        // Get cutoff values from input
        for (int32_t i = buf_start; i < buf_stop; ++i)
            cutoffs[i] = cutoff_buf[i];
    }
    else
    {
        // Get our default cutoff
        const float def_cutoff = (float)fimpl->def_cutoff;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            cutoffs[i] = def_cutoff;
    }

    float* resonances = Work_buffers_get_buffer_contents_mut(wbs, CONTROL_WB_RESONANCE);

    if (resonance_buf != NULL)
    {
        // Get resonance values from input
        for (int32_t i = buf_start; i < buf_stop; ++i)
            resonances[i] = resonance_buf[i];
    }
    else
    {
        // Get our default resonance
        const float def_resonance = (float)fimpl->def_resonance;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            resonances[i] = def_resonance;
    }

    const double max_true_cutoff_change = 0.01;
    const double max_resonance_change = 0.01;

    fimpl->filter_xfade_update = get_xfade_step(
            audio_rate, fimpl->true_cutoff, fimpl->applied_resonance);

    const double nyquist = (double)audio_rate * 0.5;

    int32_t apply_filter_start = buf_start;
    int32_t apply_filter_stop = buf_stop;
    double xfade_start = fimpl->filter_xfade_pos;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float cutoff = cutoffs[i];
        const float resonance = resonances[i];

        // TODO: apply force->filter envelope if applicable
        /*
        if (proc->au_params->env_force_filter_enabled &&
                fimpl->filter_xfade_pos >= 1)
        {
            double force = Cond_work_buffer_get_value(actual_forces, i);
            if (force > 1)
                force = 1;

            double factor = Envelope_get_value(proc->au_params->env_force_filter, force);
            rassert(isfinite(factor));
            cutoff += (factor * 100) - 100;
        }
        // */

        // Initialise new filter settings if needed
        if (fimpl->filter_xfade_pos >= 1 &&
                ((fimpl->applied_type != fimpl->type) ||
                 (fabs(cutoff - fimpl->applied_cutoff) >
                    max_true_cutoff_change) ||
                 (fabs(resonance - fimpl->applied_resonance) >
                    max_resonance_change)))
        {
            // Apply previous filter settings to the signal
            apply_filter_stop = i;
            Filter_state_impl_apply_filter_settings(
                    fimpl,
                    in_buffers,
                    out_buffers,
                    xfade_start,
                    fimpl->filter_xfade_update,
                    apply_filter_start,
                    apply_filter_stop);

            // Set up new range for next filter processing
            apply_filter_start = i;
            apply_filter_stop = buf_stop;

            fimpl->filter_xfade_state_used = fimpl->filter_state_used;

            // Only apply crossfade if we have rendered audio
            if (fimpl->anything_rendered)
                fimpl->filter_xfade_pos = 0;
            else
                fimpl->filter_xfade_pos = 1;

            fimpl->applied_type = fimpl->type;

            fimpl->applied_cutoff = cutoff;
            fimpl->true_cutoff = get_cutoff_freq(fimpl->applied_cutoff);
            if (fimpl->applied_type == FILTER_TYPE_HIGHPASS)
                fimpl->true_cutoff = min(fimpl->true_cutoff, nyquist - 1.0);

            fimpl->applied_resonance = resonance;
            fimpl->true_resonance = get_resonance(fimpl->applied_resonance);

            fimpl->filter_xfade_update = get_xfade_step(
                    audio_rate, fimpl->true_cutoff, fimpl->applied_resonance);

            if (fimpl->true_cutoff < nyquist)
            {
                const int new_state = 1 - abs(fimpl->filter_state_used);
                const double true_cutoff = max(fimpl->true_cutoff, 1);
                const int bandform =
                    (fimpl->applied_type == FILTER_TYPE_HIGHPASS) ? 1 : 0;
                fimpl->sf_state[new_state].type = fimpl->applied_type;
                two_pole_filter_create(
                        true_cutoff / audio_rate,
                        fimpl->true_resonance,
                        bandform,
                        fimpl->sf_state[new_state].coeffs,
                        &fimpl->sf_state[new_state].mul);
                for (int a = 0; a < KQT_BUFFERS_MAX; ++a)
                {
                    for (int k = 0; k < FILTER_ORDER; ++k)
                    {
                        fimpl->sf_state[new_state].history1[a][k] = 0;
                        fimpl->sf_state[new_state].history2[a][k] = 0;
                    }
                }
                fimpl->filter_state_used = new_state;
                //fprintf(stderr, "created filter with cutoff %f\n", cutoff);
            }
            else
            {
                if (fimpl->filter_state_used == -1)
                    fimpl->filter_xfade_pos = 1;

                fimpl->filter_state_used = -1;
            }

            xfade_start = fimpl->filter_xfade_pos;
        }

        fimpl->anything_rendered = true;

        fimpl->filter_xfade_pos += fimpl->filter_xfade_update;
    }

    // Apply previous filter settings to the remaining signal
    Filter_state_impl_apply_filter_settings(
            fimpl,
            in_buffers,
            out_buffers,
            xfade_start,
            fimpl->filter_xfade_update,
            apply_filter_start,
            apply_filter_stop);

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
    const float* cutoff_buf = Device_thread_state_get_mixed_buffer_contents_mut(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    const float* resonance_buf = Device_thread_state_get_mixed_buffer_contents_mut(
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

    Filter_state_impl_apply_input_buffers(
            &fpstate->state_impl,
            cutoff_buf,
            resonance_buf,
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

    const Proc_filter* filter = (const Proc_filter*)dstate->device->dimpl;

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    fpstate->state_impl.type = filter->type;

    return true;
}


bool Filter_pstate_set_cutoff(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    fpstate->state_impl.def_cutoff = value;

    return true;
}


bool Filter_pstate_set_resonance(
        Device_state* dstate, const Key_indices indices, double value)
{
    rassert(dstate != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Filter_pstate* fpstate = (Filter_pstate*)dstate;
    fpstate->state_impl.def_resonance = value;

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
    const float* cutoff_buf = Device_thread_state_get_voice_buffer_contents(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_CUTOFF);
    const float* resonance_buf = Device_thread_state_get_voice_buffer_contents(
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
    Filter_state_impl_apply_input_buffers(
            &fvstate->state_impl,
            cutoff_buf,
            resonance_buf,
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


