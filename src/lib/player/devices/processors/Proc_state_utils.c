

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Proc_state_utils.h>

#include <debug/assert.h>
#include <init/devices/Processor.h>
#include <intrinsics.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <stdlib.h>


#define RAMP_ATTACK_TIME (500.0)


Proc_state* new_Proc_state_default(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Proc_state* pstate = memory_alloc_item(Proc_state);
    if ((pstate == NULL) ||
            !Proc_state_init(pstate, device, audio_rate, audio_buffer_size))
    {
        del_Device_state(&pstate->parent);
        return NULL;
    }

    return pstate;
}


void Proc_ramp_attack(
        Voice_state* vstate,
        int buf_count,
        Work_buffer* out_wbs[buf_count],
        int32_t frame_count,
        int32_t audio_rate)
{
    rassert(vstate != NULL);
    rassert(buf_count > 0);
    rassert(out_wbs != NULL);
    rassert(frame_count > 0);
    rassert(audio_rate > 0);

    float start_ramp_attack = (float)vstate->ramp_attack;
    const float inc = (float)(RAMP_ATTACK_TIME / audio_rate);

    for (int ch = 0; ch < buf_count; ++ch)
    {
        if (out_wbs[ch] == NULL)
            continue;

        float ramp_attack = start_ramp_attack;
        float* out = Work_buffer_get_contents_mut(out_wbs[ch]);

        for (int32_t i = 0; (i < frame_count) && (ramp_attack < 1); ++i)
        {
            *out++ *= ramp_attack;
            ramp_attack += inc;
        }

        vstate->ramp_attack = ramp_attack;
    }

#if 0
    for (int ch = 0; ch < buf_count; ++ch)
    {
        if (out_bufs[ch] == NULL)
            continue;

        float ramp_attack = start_ramp_attack;

        for (int32_t i = buf_start; (i < buf_stop) && (ramp_attack < 1); ++i)
        {
            out_bufs[ch][i] *= ramp_attack;
            ramp_attack += inc;
        }

        vstate->ramp_attack = ramp_attack;
    }
#endif

    return;
}


void Proc_fill_freq_buffer(
        Work_buffer* freqs,
        Work_buffer* pitches,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(freqs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    if (Work_buffer_is_valid(pitches))
    {
        Proc_clamp_pitch_values(pitches, buf_start, buf_stop);

        const int32_t const_start = Work_buffer_get_const_start(pitches);
        float* freqs_data = Work_buffer_get_contents_mut(freqs);

        float* pitches_data = Work_buffer_get_contents_mut(pitches);

        const int32_t fast_stop = clamp(const_start, buf_start, buf_stop);

        for (int32_t i = buf_start; i < fast_stop; ++i)
            freqs_data[i] = (float)fast_cents_to_Hz(pitches_data[i]);

        //fprintf(stdout, "%d %d %d\n", (int)buf_start, (int)fast_stop, (int)buf_stop);

        if (fast_stop < buf_stop)
        {
            const float pitch = pitches_data[fast_stop];
            const float freq = isfinite(pitch) ? (float)cents_to_Hz(pitch) : 0.0f;
            for (int32_t i = fast_stop; i < buf_stop; ++i)
                freqs_data[i] = freq;
        }

        Work_buffer_set_const_start(freqs, const_start);
    }
    else
    {
        float* freqs_data = Work_buffer_get_contents_mut(freqs);

        for (int32_t i = buf_start; i < buf_stop; ++i)
            freqs_data[i] = 440;

        Work_buffer_set_const_start(freqs, buf_start);
    }

    return;
}


void Proc_fill_scale_buffer(Work_buffer* scales, Work_buffer* dBs, int32_t frame_count)
{
    rassert(scales != NULL);
    rassert(frame_count > 0);

    if (Work_buffer_is_valid(dBs))
    {
        const int32_t const_start = Work_buffer_get_const_start(dBs);
        float* scales_data = Work_buffer_get_contents_mut(scales);

        float* dBs_data = Work_buffer_get_contents_mut(dBs);

        const int32_t fast_stop = min(const_start, frame_count);

        float const_dB = -INFINITY;

#if KQT_SSE4_1
        {
            if (fast_stop < frame_count)
                const_dB = clamp(dBs_data[fast_stop], -10000.0f, 10000.0f);

            const __m128 min_dB = _mm_set1_ps(-500);
            const __m128 max_dB = _mm_set1_ps(500);

            for (int32_t i = 0; i < fast_stop; i += 4)
            {
                const __m128 dB = _mm_load_ps(dBs_data + i);
                const __m128 clamped_dB = _mm_min_ps(_mm_max_ps(min_dB, dB), max_dB);
                const __m128 scale = fast_dB_to_scale_f4(clamped_dB);
                _mm_store_ps(scales_data + i, scale);
            }
        }
#else
        // Sanitise input values
        {
            const float bound = 10000.0f;

            for (int32_t i = 0; i < fast_stop; ++i)
                dBs_data[i] = clamp(dBs_data[i], -bound, bound);

            if (fast_stop < frame_count)
                const_dB = clamp(dBs_data[fast_stop], -bound, bound);
        }

        for (int32_t i = 0; i < fast_stop; ++i)
            scales_data[i] = (float)fast_dB_to_scale(dBs_data[i]);
#endif

        //fprintf(stdout, "%d %d %d\n", 0, (int)fast_stop, (int)frame_count);

        if (fast_stop < frame_count)
        {
            rassert(isfinite(const_dB));
            const float scale = (float)dB_to_scale(const_dB);
            for (int32_t i = fast_stop; i < frame_count; ++i)
                scales_data[i] = scale;
        }

        Work_buffer_set_const_start(scales, const_start);
    }
    else
    {
        float* scales_data = Work_buffer_get_contents_mut(scales);

        for (int32_t i = 0; i < frame_count; ++i)
            scales_data[i] = 1;

        Work_buffer_set_const_start(scales, 0);
    }

    return;
}


void Proc_clamp_pitch_values(Work_buffer* pitches, int32_t buf_start, int32_t buf_stop)
{
    rassert(pitches != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    const int32_t const_start = Work_buffer_get_const_start(pitches);
    const int32_t fast_stop = clamp(const_start, buf_start, buf_stop);

    const float bound = 2000000.0f;

    float* pitches_data = Work_buffer_get_contents_mut(pitches);

    for (int32_t i = buf_start; i < fast_stop; ++i)
        pitches_data[i] = clamp(pitches_data[i], -bound, bound);

    if (fast_stop < buf_stop)
    {
        const float pitch = clamp(pitches_data[fast_stop], -bound, bound);
        for (int32_t i = fast_stop; i < buf_stop; ++i)
            pitches_data[i] = pitch;
    }

    Work_buffer_set_const_start(pitches, const_start);

    return;
}


Cond_work_buffer* Cond_work_buffer_init(
        Cond_work_buffer* cwb, const Work_buffer* wb, float def_value)
{
    rassert(cwb != NULL);

    cwb->index_mask = 0;
    cwb->def_value = def_value;
    cwb->wb_contents = &cwb->def_value;

    if (Work_buffer_is_valid(wb))
    {
        cwb->index_mask = ~(int32_t)0;
        cwb->wb_contents = Work_buffer_get_contents(wb);
    }

    return cwb;
}


