

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Ks_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_ks.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <player/devices/processors/Filter.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#define DAMP_FILTER_ORDER 1


typedef struct Damp
{
    double cutoff_norm; // used for calculating phase delay
    double coeffs[DAMP_FILTER_ORDER];
    double mul;
    double history1[DAMP_FILTER_ORDER];
    double history2[DAMP_FILTER_ORDER];
} Damp;


typedef struct Frac_delay
{
    float eta;
    float prev_item;
    float feedback;
} Frac_delay;


typedef struct Read_state
{
    int32_t read_pos;
    double pitch;
    Damp damp;
    Frac_delay frac_delay;
} Read_state;


static void Read_state_clear(Read_state* rs)
{
    rassert(rs != NULL);

    rs->read_pos = 0;
    rs->pitch = NAN;

    rs->damp.mul = 0;

    for (int i = 0; i < DAMP_FILTER_ORDER; ++i)
    {
        rs->damp.coeffs[i] = 0;
        rs->damp.history1[i] = 0;
        rs->damp.history2[i] = 0;
    }

    rs->frac_delay.eta = 0;
    rs->frac_delay.prev_item = 0;
    rs->frac_delay.feedback = 0;

    return;
}


static void Read_state_modify(
        Read_state* rs,
        double pitch,
        int32_t write_pos,
        int32_t buf_len,
        int32_t audio_rate)
{
    rassert(rs != NULL);
    rassert(isfinite(pitch));
    rassert(write_pos >= 0);
    rassert(buf_len > 2);
    rassert(write_pos < buf_len);
    rassert(audio_rate > 0);

    const double freq = cents_to_Hz(pitch);
    const double period_length = audio_rate / freq;
    const double used_buf_length = clamp(period_length, 2, buf_len);
    const int32_t used_buf_frames_whole = (int32_t)floor(used_buf_length);

    rs->read_pos = (buf_len + write_pos - used_buf_frames_whole) % buf_len;
    rs->pitch = pitch;

    // Phase delay for one-pole lowpass is: atan(tan(pi*f)/tan(pi*f0))/(2*pi*f)
    const double freq_norm = freq / audio_rate;
    const double delay_add =
        -atan(tan(PI * freq_norm)/tan(PI * rs->damp.cutoff_norm)) / (2 * PI * freq_norm);

    // Set up fractional delay filter
    float delay = (float)(used_buf_length - used_buf_frames_whole) + (float)delay_add;
    while (delay < 0.618f)
    {
        delay += 1.0f;
        rs->read_pos = (rs->read_pos + 1) % buf_len;
    }

    rs->frac_delay.eta = (1 - delay) / (1 + delay);

    return;
}


#define CUTOFF_BIAS 74.37631656229591


static void Read_state_init(
        Read_state* rs,
        double damp,
        double pitch,
        int32_t write_pos,
        int32_t buf_len,
        int32_t audio_rate)
{
    rassert(rs != NULL);
    rassert(damp >= 0);
    rassert(damp <= 100);
    rassert(isfinite(pitch));
    rassert(write_pos >= 0);
    rassert(buf_len > 2);
    rassert(write_pos < buf_len);
    rassert(audio_rate > 0);

    Read_state_clear(rs);

    // Set up damping filter
    const double cutoff = exp2((100 - damp + CUTOFF_BIAS) / 12.0);
    const double cutoff_clamped = clamp(cutoff, 1, (audio_rate / 2) - 1);
    rs->damp.cutoff_norm = cutoff_clamped / audio_rate;
    one_pole_filter_create(
            rs->damp.cutoff_norm, 0, rs->damp.coeffs, &rs->damp.mul);

    Read_state_modify(rs, pitch, write_pos, buf_len, audio_rate);

    return;
}


static float Read_state_update(
        Read_state* rs,
        float excitation,
        int32_t delay_buf_len,
        const float delay_buf[delay_buf_len])
{
    rassert(rs != NULL);
    rassert(delay_buf_len > 2);
    rassert(delay_buf != NULL);

    const float src_value = excitation + delay_buf[rs->read_pos];

    // Apply damping filter
    double damped = nq_zero_filter(
            DAMP_FILTER_ORDER, rs->damp.history1, src_value);
    damped = iir_filter_strict_cascade(
            DAMP_FILTER_ORDER, rs->damp.coeffs, rs->damp.history2, damped);
    damped *= rs->damp.mul;

    // Apply fractional delay filter
    Frac_delay* fd = &rs->frac_delay;
    const float value =
        fd->eta * (float)damped + fd->prev_item - fd->eta * fd->feedback;
    fd->prev_item = (float)damped;
    fd->feedback = value;

    ++rs->read_pos;
    if (rs->read_pos >= delay_buf_len)
        rs->read_pos = 0;

    return value;
}


static void Read_state_copy(Read_state* restrict dest, const Read_state* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);

    *dest = *src;

    return;
}


typedef struct Ks_vstate
{
    Voice_state parent;

    int32_t write_pos;
    int primary_read_state;
    bool is_xfading;
    double xfade_progress;
    Read_state read_states[2];
} Ks_vstate;


int32_t Ks_vstate_get_size(void)
{
    return sizeof(Ks_vstate);
}


enum
{
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_EXCITATION,
    PORT_IN_COUNT
};


enum
{
    PORT_OUT_AUDIO = 0,
    PORT_OUT_COUNT
};


static const int KS_WB_FIXED_PITCH = WORK_BUFFER_IMPL_1;
static const int KS_WB_FIXED_FORCE = WORK_BUFFER_IMPL_2;
static const int KS_WB_FIXED_EXCITATION = WORK_BUFFER_IMPL_3;


static int32_t Ks_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);
    rassert(tempo > 0);

    if (buf_start == buf_stop)
        return buf_start;

    const Device_state* dstate = &proc_state->parent;

    Ks_vstate* ks_vstate = (Ks_vstate*)vstate;

    // Get frequencies
    const Work_buffer* pitches_wb = Proc_state_get_voice_buffer(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_PITCH);
    if (pitches_wb == NULL)
    {
        Work_buffer* fixed_pitches_wb =
            Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_PITCH);
        Work_buffer_clear(fixed_pitches_wb, buf_start, buf_stop);
        pitches_wb = fixed_pitches_wb;
    }
    const float* pitches = Work_buffer_get_contents(pitches_wb);

    // Get volume scales
    Work_buffer* scales_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_FORCE);
    const Work_buffer* dBs_wb = scales_wb;
    if (scales_wb == NULL)
        scales_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_FORCE);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, buf_start, buf_stop);
    const float* scales = Work_buffer_get_contents(scales_wb);

    // Get excitation signal
    const Work_buffer* excit_wb = Proc_state_get_voice_buffer(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_EXCITATION);
    if (excit_wb == NULL)
    {
        Work_buffer* fixed_excit_wb =
            Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_EXCITATION);
        Work_buffer_clear(fixed_excit_wb, buf_start, buf_stop);
        excit_wb = fixed_excit_wb;
    }
    const float* excits = Work_buffer_get_contents(excit_wb);

    // Get output buffer for writing
    float* out_buf = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO);
    if (out_buf == NULL)
        return buf_start;

    const bool need_init =
        isnan(ks_vstate->read_states[ks_vstate->primary_read_state].pitch);

    // Get delay buffer
    Work_buffer* delay_wb = ks_vstate->parent.wb;
    rassert(delay_wb != NULL);
    const int32_t delay_wb_size = Work_buffer_get_size(delay_wb);

    const int32_t audio_rate = dstate->audio_rate;

    if (need_init)
    {
        ks_vstate->write_pos = 0;

        const Proc_ks* ks = (const Proc_ks*)proc_state->parent.device->dimpl;

        Read_state_init(
                &ks_vstate->read_states[ks_vstate->primary_read_state],
                ks->damp,
                pitches[buf_start],
                ks_vstate->write_pos,
                delay_wb_size,
                audio_rate);
    }

    int32_t write_pos = ks_vstate->write_pos;
    float* delay_buf = Work_buffer_get_contents_mut(delay_wb);

    static const double xfade_speed = 1000.0;
    const double xfade_step = xfade_speed / audio_rate;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float pitch = pitches[i];
        const float scale = scales[i];
        const float excitation = excits[i];

        if (!ks_vstate->is_xfading)
        {
            const int32_t const_start = Work_buffer_get_const_start(pitches_wb);
            const float max_diff = (i < const_start) ? 4.0f : 0.001f;

            Read_state* primary_rs =
                &ks_vstate->read_states[ks_vstate->primary_read_state];
            if (fabs(pitch - primary_rs->pitch) > max_diff)
            {
                Read_state* other_rs =
                    &ks_vstate->read_states[1 - ks_vstate->primary_read_state];

                // Instantaneous slides to lower pitches don't work very well,
                // so limit the step length when sliding downwards
                const double min_pitch = primary_rs->pitch - 200.0;
                const double cur_target_pitch = max(min_pitch, pitch);

                Read_state_copy(other_rs, primary_rs);
                Read_state_modify(
                        other_rs,
                        cur_target_pitch,
                        write_pos,
                        delay_wb_size,
                        audio_rate);

                ks_vstate->primary_read_state = 1 - ks_vstate->primary_read_state;
                ks_vstate->is_xfading = true;
                ks_vstate->xfade_progress = 0.0;
            }
        }

        float value = 0.0f;
        if (!ks_vstate->is_xfading)
        {
            value = Read_state_update(
                    &ks_vstate->read_states[ks_vstate->primary_read_state],
                    excitation,
                    delay_wb_size,
                    delay_buf);
        }
        else
        {
            const float out_value = Read_state_update(
                    &ks_vstate->read_states[1 - ks_vstate->primary_read_state],
                    excitation,
                    delay_wb_size,
                    delay_buf);
            const float in_value = Read_state_update(
                    &ks_vstate->read_states[ks_vstate->primary_read_state],
                    excitation,
                    delay_wb_size,
                    delay_buf);

            value = lerp(out_value, in_value, (float)ks_vstate->xfade_progress);

            ks_vstate->xfade_progress += xfade_step;
            if (ks_vstate->xfade_progress >= 1.0)
                ks_vstate->is_xfading = false;
        }

        out_buf[i] = value * scale;

        delay_buf[write_pos] = value;

        ++write_pos;
        if (write_pos >= delay_wb_size)
            write_pos = 0;
    }

    ks_vstate->write_pos = write_pos;

    return buf_stop;
}


void Ks_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Ks_vstate_render_voice;

    Ks_vstate* ks_vstate = (Ks_vstate*)vstate;

    ks_vstate->write_pos = 0;
    ks_vstate->primary_read_state = 0;
    ks_vstate->is_xfading = false;
    ks_vstate->xfade_progress = 0.0;

    Read_state_clear(&ks_vstate->read_states[0]);
    Read_state_clear(&ks_vstate->read_states[1]);

    Work_buffer* delay_wb = ks_vstate->parent.wb;
    rassert(delay_wb != NULL);
    Work_buffer_clear(delay_wb, 0, Work_buffer_get_size(delay_wb));

    return;
}


