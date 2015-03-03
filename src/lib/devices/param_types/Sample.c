

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <debug/assert.h>
#include <devices/generators/Generator_common.h>
#include <devices/param_types/Sample.h>
#include <devices/param_types/Sample_mix.h>
#include <devices/param_types/Sample_params.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffers.h>


Sample* new_Sample(void)
{
    Sample* sample = memory_alloc_item(Sample);
    if (sample == NULL)
        return NULL;

//    Sample_params_init(&sample->params);
//    sample->path = NULL;
//    sample->changed = false;
//    sample->is_lossy = false;
    sample->channels = 1;
    sample->bits = 16;
    sample->is_float = false;
    sample->len = 0;
    sample->data[0] = NULL;
    sample->data[1] = NULL;

    return sample;
}


Sample* new_Sample_from_buffers(float* buffers[], int count, uint64_t length)
{
    assert(buffers != NULL);
    assert(count >= 1);
    assert(count <= 2);
    assert(length > 0);

    Sample* sample = new_Sample();
    if (sample == NULL)
        return NULL;

    sample->channels = count;
    sample->bits = 32;
    sample->is_float = true;
    sample->len = length;
    for (int i = 0; i < count; ++i)
    {
        assert(buffers[i] != NULL);
        sample->data[i] = buffers[i];
    }

    return sample;
}


void* Sample_get_buffer(Sample* sample, int ch)
{
    assert(sample != NULL);
    assert(ch >= 0);
    assert(ch < sample->channels);

    return sample->data[ch];
}


uint32_t Sample_mix(
        const Sample* sample,
        const Sample_params* params,
        const Generator* gen,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo,
        double middle_tone,
        double middle_freq,
        double vol_scale)
{
    assert(sample != NULL);
    assert(params != NULL);
    assert(gen != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(vol_scale >= 0);
    (void)gen;
    (void)ins_state;
    (void)tempo;

    // This implementation does not support larger sample lengths :-P
    assert(sample->len < INT32_MAX - 1);

    if (sample->len == 0)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get work buffers
    const float* actual_pitches = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const float* actual_forces = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_FORCES);

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L),
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R),
    };

    static const int SAMPLE_WORK_BUFFER_POSITIONS = WORK_BUFFER_IMPL_1;
    static const int SAMPLE_WORK_BUFFER_NEXT_POSITIONS = WORK_BUFFER_IMPL_2;
    static const int SAMPLE_WORK_BUFFER_POSITIONS_REM = WORK_BUFFER_IMPL_3;

    int32_t* positions = Work_buffers_get_buffer_contents_int_mut(
            wbs, SAMPLE_WORK_BUFFER_POSITIONS);
    int32_t* next_positions = Work_buffers_get_buffer_contents_int_mut(
            wbs, SAMPLE_WORK_BUFFER_NEXT_POSITIONS);
    float* positions_rem = Work_buffers_get_buffer_contents_mut(
            wbs, SAMPLE_WORK_BUFFER_POSITIONS_REM);

    // Position information to be updated
    int32_t new_pos = vstate->pos;
    double new_pos_rem = vstate->pos_rem;

    // Get sample positions (assuming no loop at this point)
    const double shift_factor = middle_freq / (middle_tone * audio_rate);
    positions[buf_start] = new_pos;
    positions_rem[buf_start] = new_pos_rem;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_pitch = actual_pitches[i];
        const double shift_total = actual_pitch * shift_factor;

        const int32_t shift_floor = floor(shift_total);
        const double shift_rem = shift_total - shift_floor;

        new_pos += shift_floor;
        new_pos_rem += shift_rem;
        const int32_t excess_whole = floor(new_pos_rem);
        new_pos += excess_whole;
        new_pos_rem -= excess_whole;

        positions[i + 1] = new_pos;
        positions_rem[i + 1] = new_pos_rem;
    }

    // Prevent invalid loop processing
    Sample_loop loop_mode = params->loop;
    if ((params->loop_end > sample->len) || (params->loop_start >= params->loop_end))
        loop_mode = SAMPLE_LOOP_OFF;

    // Apply loop and length constraints to sample positions
    int32_t new_buf_stop = buf_stop;
    switch (loop_mode)
    {
        case SAMPLE_LOOP_OFF:
        {
            const int32_t length = (int32_t)sample->len;

            // Current positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                if (positions[i] >= length)
                {
                    new_buf_stop = i;

                    // Make the index safe to access
                    positions[i] = length - 1;

                    vstate->active = false;

                    break;
                }
            }

            // Next positions
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                next_positions[i] = positions[i] + 1;
            if (buf_start < new_buf_stop)
            {
                const int32_t last_index = new_buf_stop - 1;
                next_positions[last_index] = min(length - 1, next_positions[last_index]);
            }
        }
        break;

        case SAMPLE_LOOP_UNI:
        {
            const int32_t loop_start = params->loop_start;
            const int32_t loop_end = params->loop_end;
            const int32_t loop_length = loop_end - loop_start;

            // Current positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t cur_pos = positions[i];
                if (cur_pos > loop_start)
                {
                    int32_t loop_pos = cur_pos - loop_start;
                    loop_pos %= loop_length;
                    cur_pos = loop_start + loop_pos;
                    positions[i] = cur_pos;
                }
            }

            // Next positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t next_pos = positions[i] + 1;
                if (next_pos >= loop_end)
                    next_pos = loop_start;
                next_positions[i] = next_pos;
            }
        }
        break;

        case SAMPLE_LOOP_BI:
        {
            const int32_t loop_start = params->loop_start;
            const int32_t uni_loop_length = params->loop_end - loop_start - 1;
            const int32_t step_count = uni_loop_length * 2;
            const int32_t loop_length = max(1, step_count);

            // Current positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t cur_pos = positions[i];
                if (cur_pos > loop_start)
                {
                    int32_t loop_pos = cur_pos - loop_start;
                    loop_pos %= loop_length;
                    if (loop_pos >= uni_loop_length)
                        loop_pos = step_count - loop_pos;
                    cur_pos = loop_start + loop_pos;
                    positions[i] = cur_pos;
                    assert(cur_pos >= 0);
                }
            }

            // Next positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t next_pos = positions[i] + 1;
                if (next_pos > loop_start)
                {
                    int32_t next_loop_pos = next_pos - loop_start;
                    next_loop_pos %= loop_length;
                    if (next_loop_pos >= uni_loop_length)
                        next_loop_pos = step_count - next_loop_pos;
                    next_pos = loop_start + next_loop_pos;
                }
                next_positions[i] = next_pos;
            }
        }
        break;

        default:
            assert(false);
    }

    // Get sample frames
#define get_item(out_value)                             \
    if (true)                                           \
    {                                                   \
        const int32_t cur_pos = positions[i];           \
        const int32_t next_pos = next_positions[i];     \
        const float lerp_value = positions_rem[i];      \
                                                        \
        const float cur_value = data[cur_pos];          \
        const float next_value = data[next_pos];        \
        const float diff = next_value - cur_value;      \
        (out_value) = cur_value + (lerp_value * diff);  \
    }                                                   \
    else (void)0

    if (!sample->is_float)
    {
        switch (sample->bits)
        {
            case 8:
            {
                static const double scale = 1.0 / 0x80;
                const double fixed_scale = vol_scale * scale;
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int8_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float actual_force = actual_forces[i];
                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * actual_force * fixed_scale;
                    }
                }
            }
            break;

            case 16:
            {
                static const double scale = 1.0 / 0x8000UL;
                const double fixed_scale = vol_scale * scale;
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int16_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float actual_force = actual_forces[i];
                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * actual_force * fixed_scale;
                    }
                }
            }
            break;

            case 32:
            {
                static const double scale = 1.0 / 0x80000000UL;
                const double fixed_scale = vol_scale * scale;
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int32_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float actual_force = actual_forces[i];
                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * actual_force * fixed_scale;
                    }
                }
            }
            break;

            default:
                assert(false);
        }
    }
    else
    {
        for (int ch = 0; ch < sample->channels; ++ch)
        {
            const float* data = sample->data[ch];
            float* audio_buffer = abufs[ch];

            for (int32_t i = buf_start; i < new_buf_stop; ++i)
            {
                const float actual_force = actual_forces[i];
                float item = 0;
                get_item(item);
                audio_buffer[i] = item * actual_force * vol_scale;
            }
        }
    }

#undef get_item

    // Copy mono signal to the right channel
    if (sample->channels == 1)
    {
        const int32_t frame_count = new_buf_stop - buf_start;
        memcpy(abufs[1] + buf_start, abufs[0] + buf_start, sizeof(float) * frame_count);
    }

    // Update position information
    vstate->pos = new_pos;
    vstate->pos_rem = new_pos_rem;

    return new_buf_stop;
}


uint64_t Sample_get_len(const Sample* sample)
{
    assert(sample != NULL);
    return sample->len;
}


void del_Sample(Sample* sample)
{
    if (sample == NULL)
        return;

    memory_free(sample->data[0]);
    memory_free(sample->data[1]);
    memory_free(sample);

    return;
}


