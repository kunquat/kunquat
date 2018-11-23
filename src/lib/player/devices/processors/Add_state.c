

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Add_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_add.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct Add_tone_state
{
    double phase[2];
} Add_tone_state;


typedef struct Add_vstate
{
    Voice_state parent;
    int tone_limit;
    float prev_mod[2];
    Add_tone_state tones[ADD_TONES_MAX];
} Add_vstate;


int32_t Add_vstate_get_size(void)
{
    return sizeof(Add_vstate);
}


enum
{
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_PHASE_MOD_L,
    PORT_IN_PHASE_MOD_R,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int ADD_WORK_BUFFER_FIXED_PITCH = WORK_BUFFER_IMPL_1;
static const int ADD_WORK_BUFFER_FIXED_FORCE = WORK_BUFFER_IMPL_2;
static const int ADD_WORK_BUFFER_MOD_L = WORK_BUFFER_IMPL_3;
//static const int ADD_WORK_BUFFER_MOD_R = WORK_BUFFER_IMPL_4;


int32_t Add_vstate_render_voice(
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
    rassert(tempo > 0);

    const Device_state* dstate = &proc_state->parent;
    const Proc_add* add = (Proc_add*)proc_state->parent.device->dimpl;
    Add_vstate* add_state = (Add_vstate*)vstate;
    rassert(is_p2(ADD_BASE_FUNC_SIZE));

    // Get frequencies
    Work_buffer* freqs_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PITCH, NULL);
    Work_buffer* pitches_wb = freqs_wb;
    if ((freqs_wb == NULL) || !Work_buffer_is_valid(freqs_wb, 0))
        freqs_wb = Work_buffers_get_buffer_mut(wbs, ADD_WORK_BUFFER_FIXED_PITCH, 1);

    Proc_fill_freq_buffer(freqs_wb, pitches_wb, buf_start, buf_stop);
    const float* freqs = Work_buffer_get_contents(freqs_wb, 0);

    // Get volume scales
    Work_buffer* scales_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE, NULL);
    Work_buffer* dBs_wb = scales_wb;
    if ((dBs_wb != NULL) &&
            Work_buffer_is_valid(dBs_wb, 0) &&
            Work_buffer_is_final(dBs_wb, 0) &&
            (Work_buffer_get_const_start(dBs_wb, 0) <= buf_start) &&
            (Work_buffer_get_contents(dBs_wb, 0)[buf_start] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        vstate->active = false;
        return buf_start;
    }

    if (scales_wb == NULL)
        scales_wb = Work_buffers_get_buffer_mut(wbs, ADD_WORK_BUFFER_FIXED_FORCE, 1);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, buf_start, buf_stop);
    const float* scales = Work_buffer_get_contents(scales_wb, 0);

    // Get output buffer for writing
    float* out_bufs[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_ts, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_bufs);
    for (int ch = 0; ch < 2; ++ch)
    {
        float* out_buf = out_bufs[ch];
        if (out_buf != NULL)
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_buf[i] = 0;
        }
    }

    // Get phase modulation signal
    const Work_buffer* mod_wbs[] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PHASE_MOD_L, NULL),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PHASE_MOD_R, NULL),
    };

    for (int ch = 0; ch < 2; ++ch)
    {
        if ((mod_wbs[ch] == NULL) || !Work_buffer_is_valid(mod_wbs[ch], 0))
        {
            Work_buffer* zero_buf = Work_buffers_get_buffer_mut(
                    wbs, (Work_buffer_type)(ADD_WORK_BUFFER_MOD_L + ch), 1);
            Work_buffer_clear(zero_buf, 0, buf_start, buf_stop);
            mod_wbs[ch] = zero_buf;
        }
    }

    // Add base waveform tones
    const double inv_audio_rate = 1.0 / dstate->audio_rate;

    const float* base = Sample_get_buffer(add->base, 0);

    for (int h = 0; h < add_state->tone_limit; ++h)
    {
        const Add_tone* tone = &add->tones[h];
        const double pitch_factor = tone->pitch_factor;
        const double volume_factor = tone->volume_factor;

        if ((pitch_factor <= 0) || (volume_factor <= 0))
            continue;

        const double pannings[] =
        {
            -tone->panning,
            tone->panning,
        };

        const double pitch_factor_inv_audio_rate = pitch_factor * inv_audio_rate;

        Add_tone_state* tone_state = &add_state->tones[h];

        for (int32_t ch = 0; ch < 2; ++ch)
        {
            float* out_buf_ch = out_bufs[ch];
            if (out_buf_ch == NULL)
                continue;

            const double panning_factor = 1 + pannings[ch];
            const float* mod_values_ch = Work_buffer_get_contents(mod_wbs[ch], 0);

            double phase = tone_state->phase[ch];

            int32_t res_slice_start = buf_start;
            while (res_slice_start < buf_stop)
            {
                int32_t res_slice_stop = buf_stop;

                // Get current pitch range
                const float first_mod_shift =
                    mod_values_ch[res_slice_start] - add_state->prev_mod[ch];
                const float first_phase_shift_abs = (float)fabs(
                        first_mod_shift +
                        (freqs[res_slice_start] * pitch_factor_inv_audio_rate));
                int shift_exp = 0;
                const float shift_norm = frexpf(first_phase_shift_abs, &shift_exp);
                const float min_phase_shift_abs = ldexpf(0.5f, shift_exp);
                const float max_phase_shift_abs = min_phase_shift_abs * 2.0f;

                // Choose appropriate waveform resolution for current pitch range
                int32_t cur_size = ADD_BASE_FUNC_SIZE;
                if (isfinite(shift_norm) && (shift_norm > 0.0f))
                {
                    cur_size = (int32_t)ipowi(2, clamp(-shift_exp + 1, 3, 30));
                    cur_size = min(cur_size, ADD_BASE_FUNC_SIZE * 2);
                    rassert(is_p2(cur_size));
                }
                const uint32_t cur_size_mask = (uint32_t)cur_size - 1;
                const int base_offset = (ADD_BASE_FUNC_SIZE * 4 - cur_size * 2);
                rassert(base_offset >= 0);
                rassert(base_offset < (ADD_BASE_FUNC_SIZE * 4) - 1);
                const float* cur_base = base + base_offset;

                // Get length of input compatible with current waveform resolution
                const int32_t res_check_stop = min(res_slice_stop,
                        max(Work_buffer_get_const_start(freqs_wb, 0),
                            Work_buffer_get_const_start(mod_wbs[ch], 0)) + 1);
                for (int32_t i = res_slice_start + 1; i < res_check_stop; ++i)
                {
                    const float cur_mod_shift = mod_values_ch[i] - mod_values_ch[i - 1];
                    const float cur_phase_shift_abs = (float)fabs(
                            cur_mod_shift +
                            (freqs[i] * pitch_factor_inv_audio_rate));
                    if (cur_phase_shift_abs < min_phase_shift_abs ||
                            cur_phase_shift_abs > max_phase_shift_abs)
                    {
                        res_slice_stop = i;
                        break;
                    }
                }

                for (int32_t i = res_slice_start; i < res_slice_stop; ++i)
                {
                    const float freq = freqs[i];
                    const float vol_scale = scales[i];
                    const float mod_val = mod_values_ch[i];

                    // Note: + mod_val is specific to phase modulation
                    const double actual_phase = phase + mod_val;
                    const double pos = actual_phase * cur_size;

                    // Note: direct cast of negative doubles to uint32_t is undefined
                    const uint32_t pos1 = (uint32_t)(int32_t)floor(pos) & cur_size_mask;
                    const uint32_t pos2 = (pos1 + 1) & cur_size_mask;

                    const float item1 = cur_base[pos1];
                    const float item_diff = cur_base[pos2] - item1;
                    const double lerp_val = pos - floor(pos);
                    const double value =
                        (item1 + (lerp_val * item_diff)) * volume_factor * panning_factor;

                    out_buf_ch[i] += (float)value * vol_scale;

                    phase += freq * pitch_factor_inv_audio_rate;

                    // Normalise to range [0, 1)
                    if (phase >= 1)
                    {
                        phase -= 1;

                        // Don't bother updating the phase if our frequency is too high
                        if (phase >= 1)
                            phase = tone_state->phase[ch];
                    }
                }

                rassert(res_slice_start < res_slice_stop);
                add_state->prev_mod[ch] = mod_values_ch[res_slice_stop - 1];

                res_slice_start = res_slice_stop;
            }

            tone_state->phase[ch] = phase;
        }
    }

    if (add->is_ramp_attack_enabled)
        Proc_ramp_attack(vstate, 2, out_bufs, buf_start, buf_stop, dstate->audio_rate);

    return buf_stop;
}


void Add_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Proc_add* add = (Proc_add*)proc_state->parent.device->dimpl;
    Add_vstate* add_state = (Add_vstate*)vstate;

    add_state->tone_limit = 0;

    for (int ch = 0; ch < 2; ++ch)
        add_state->prev_mod[ch] = 0;

    for (int h = 0; h < ADD_TONES_MAX; ++h)
    {
        if (add->tones[h].pitch_factor <= 0 ||
                add->tones[h].volume_factor <= 0)
            continue;

        add_state->tone_limit = h + 1;

        const double phase =
            add->is_rand_phase_enabled ? Random_get_float_lb(vstate->rand_p) : 0;

        for (int ch = 0; ch < 2; ++ch)
            add_state->tones[h].phase[ch] = phase;
    }

    return;
}


