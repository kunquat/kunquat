

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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

#include <Sample.h>
#include <Sample_params.h>
#include <Generator_common.h>
#include <File_wavpack.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


Sample* new_Sample(void)
{
    Sample* sample = xalloc(Sample);
    if (sample == NULL)
    {
        return NULL;
    }
    Sample_params_init(&sample->params);
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
    {
        return NULL;
    }
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


void Sample_set_params(Sample* sample, Sample_params* params)
{
    assert(sample != NULL);
    assert(params != NULL);
    Sample_params_copy(&sample->params, params);
    return;
}


#if 0
Sample_format Sample_get_format(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.format;
}
#endif


#if 0
char* Sample_get_path(Sample* sample)
{
    assert(sample != NULL);
    return sample->path;
}
#endif


void Sample_set_loop(Sample* sample, Sample_loop loop)
{
    assert(sample != NULL);
    assert(loop == SAMPLE_LOOP_OFF ||
           loop == SAMPLE_LOOP_UNI ||
           loop == SAMPLE_LOOP_BI);
    if (sample->len == 0)
    {
        sample->params.loop = SAMPLE_LOOP_OFF;
        return;
    }
    if (sample->params.loop_start > 0 &&
            sample->params.loop_start >= sample->params.loop_end)
    {
        sample->params.loop_start = sample->params.loop_end - 1;
    }
    if (sample->params.loop_start >= sample->len)
    {
        sample->params.loop_start = sample->len - 1;
    }
    if (sample->params.loop_end <= sample->params.loop_start)
    {
        if (sample->params.loop_start == 0)
        {
            sample->params.loop_end = sample->len;
        }
        else
        {
            sample->params.loop_end = sample->params.loop_start + 1;
        }
    }
    assert(sample->params.loop_start < sample->params.loop_end);
    assert(sample->params.loop_end <= sample->len);
    sample->params.loop = loop;
    return;
}


Sample_loop Sample_get_loop(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.loop;
}


void Sample_set_loop_start(Sample* sample, uint64_t start)
{
    assert(sample != NULL);
    if (start >= sample->len || start >= sample->params.loop_end)
    {
        sample->params.loop = SAMPLE_LOOP_OFF;
    }
    sample->params.loop_start = start;
    return;
}


uint64_t Sample_get_loop_start(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.loop_start;
}


void Sample_set_loop_end(Sample* sample, uint64_t end)
{
    assert(sample != NULL);
    if (end <= sample->params.loop_start || end >= sample->len)
    {
        sample->params.loop = SAMPLE_LOOP_OFF;
    }
    sample->params.loop_end = end;
    return;
}


uint64_t Sample_get_loop_end(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.loop_end;
}


uint32_t Sample_mix(Sample* sample,
                    Generator* gen,
                    Voice_state* state,
                    uint32_t nframes,
                    uint32_t offset,
                    uint32_t freq,
                    double tempo,
//                    int buf_count,
                    kqt_frame** bufs,
                    double middle_tone,
                    double middle_freq,
                    double vol_scale)
{
    assert(sample != NULL);
    assert(gen != NULL);
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
//    assert(buf_count > 0);
//    (void)buf_count;
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(bufs[1] != NULL);
    assert(vol_scale >= 0);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
    uint32_t mixed = offset;
    for (; mixed < nframes && state->active; ++mixed)
    {
        if (state->rel_pos >= sample->len)
        {
            state->active = false;
            break;
        }

        Generator_common_handle_pitch(gen, state);

        bool next_exists = false;
        uint64_t next_pos = 0;
        if (state->dir > 0 || sample->params.loop != SAMPLE_LOOP_BI)
        {
            uint64_t limit = sample->len;
            if (sample->params.loop)
            {
                limit = sample->params.loop_end;
            }
            if (state->rel_pos + 1 < limit)
            {
                next_exists = true;
                next_pos = state->rel_pos + 1;
            }
            else
            {
                if (sample->params.loop == SAMPLE_LOOP_UNI)
                {
                    next_exists = true;
                    next_pos = sample->params.loop_start;
                }
                else if (sample->params.loop == SAMPLE_LOOP_BI)
                {
                    next_exists = true;
                    if (state->rel_pos > sample->params.loop_start)
                    {
                        next_pos = state->rel_pos - 1;
                    }
                    else
                    {
                        next_pos = sample->params.loop_start;
                    }
                }
            }
        }
        else if (sample->params.loop_start + 1 == sample->params.loop_end)
        {
            next_exists = true;
            next_pos = sample->params.loop_start;
        }
        else
        {
            assert(sample->params.loop == SAMPLE_LOOP_BI);
            next_exists = true;
            if (state->rel_pos > sample->params.loop_start)
            {
                next_pos = state->rel_pos - 1;
            }
            else
            {
                next_pos = sample->params.loop_start + 1;
            }
            if (next_pos >= sample->params.loop_end)
            {
                next_pos = sample->params.loop_end - 1;
            }
        }
        assert(!sample->params.loop || next_pos < sample->params.loop_end);
        assert(next_pos < sample->len);
        double mix_factor = state->rel_pos_rem;
        if (next_pos < state->rel_pos)
        {
            mix_factor = 1 - state->rel_pos_rem;
        }
        double vals[KQT_BUFFERS_MAX] = { 0 };

#define get_items(type)                                         \
        if (true)                                               \
        {                                                       \
            type* buf_l = sample->data[0];                      \
            type cur[2] = { buf_l[state->rel_pos] };            \
            type next[2] = { 0 };                               \
            if (next_exists)                                    \
            {                                                   \
                next[0] = buf_l[next_pos];                      \
            }                                                   \
            vals[0] = (double)cur[0] + mix_factor *             \
                      ((double)next[0] - (double)cur[0]);       \
            if (sample->channels > 1)                           \
            {                                                   \
                type* buf_r = sample->data[1];                  \
                cur[1] = buf_r[state->rel_pos];                 \
                if (next_exists)                                \
                {                                               \
                    next[1] = buf_r[next_pos];                  \
                }                                               \
                vals[1] = (double)cur[1] + mix_factor *         \
                          ((double)next[1] - (double)cur[1]);   \
            }                                                   \
            else                                                \
            {                                                   \
                vals[1] = vals[0];                              \
            }                                                   \
        } else (void)0

        if (sample->is_float)
        {
            get_items(float);
        }
        else
        {
            switch (sample->bits)
            {
                case 8:
                {
                    get_items(int8_t);
                    vals[0] /= 0x80;
                    vals[1] /= 0x80;
                }
                break;
                case 16:
                {
                    get_items(int16_t);
                    vals[0] /= 0x8000UL;
                    vals[1] /= 0x8000UL;
                }
                break;
                case 32:
                {
                    get_items(int32_t);
                    vals[0] /= 0x80000000UL;
                    vals[1] /= 0x80000000UL;
                }
                default:
                    assert(false);
            }
        }
#undef get_items

        Generator_common_handle_force(gen, state, vals, 2, freq);
        Generator_common_handle_filter(gen, state, vals, 2, freq);
        double advance = (state->actual_pitch / middle_tone) * middle_freq / freq;
        uint64_t adv = floor(advance);
        double adv_rem = advance - adv;
        state->pos += adv;
        state->pos_rem += adv_rem;
//        Generator_common_handle_note_off(gen, state, vals, 2, freq);
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0] * vol_scale;
        bufs[1][mixed] += vals[1] * vol_scale;
        if (state->pos_rem >= 1)
        {
            state->pos += floor(state->pos_rem);
            state->pos_rem -= floor(state->pos_rem);
        }
        if (sample->params.loop == SAMPLE_LOOP_OFF)
        {
            state->rel_pos = state->pos;
            state->rel_pos_rem = state->pos_rem;
            if (state->rel_pos >= sample->len)
            {
                state->active = false;
                break;
            }
        }
        else
        {
            uint64_t loop_len = sample->params.loop_end - sample->params.loop_start;
            uint64_t limit = sample->params.loop == SAMPLE_LOOP_UNI
                                                  ? loop_len
                                                  : 2 * loop_len - 2;
            uint64_t virt_pos = state->pos;
            if (virt_pos < sample->params.loop_end)
            {
                state->dir = 1;
                state->rel_pos = state->pos;
                state->rel_pos_rem = state->pos_rem;
            }
            else if (sample->params.loop_start + 1 == sample->params.loop_end)
            {
                state->dir = 1;
                state->rel_pos = sample->params.loop_start;
                state->rel_pos_rem = 0;
            }
            else
            {
                virt_pos = ((virt_pos - sample->params.loop_start) % limit) +
                        sample->params.loop_start;
                if (sample->params.loop == SAMPLE_LOOP_UNI ||
                        virt_pos < sample->params.loop_end - 1)
                {
                    assert(sample->params.loop != SAMPLE_LOOP_UNI ||
                            virt_pos < sample->params.loop_end);
                    state->dir = 1;
                    state->rel_pos = virt_pos;
                    state->rel_pos_rem = state->pos_rem;
                }
                else
                {
                    assert(sample->params.loop == SAMPLE_LOOP_BI);
                    assert(virt_pos >= sample->params.loop_end - 1);
                    state->dir = -1;
                    uint64_t back = virt_pos - (sample->params.loop_end - 1);
                    state->rel_pos = sample->params.loop_end - 1 - back;
                    state->rel_pos_rem = state->pos_rem;
                    if (state->rel_pos > sample->params.loop_start &&
                            state->rel_pos_rem > 0)
                    {
                        --state->rel_pos;
                        state->rel_pos_rem = 1 - state->rel_pos_rem;
                    }
                }
            }
        }
        assert(state->rel_pos < sample->len);
    }
    return mixed;
}


void Sample_set_freq(Sample* sample, double freq)
{
    assert(sample != NULL);
    sample->params.mid_freq = freq;
    return;
}


double Sample_get_freq(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.mid_freq;
}


uint64_t Sample_get_len(Sample* sample)
{
    assert(sample != NULL);
    return sample->len;
}


void del_Sample(Sample* sample)
{
    if (sample == NULL)
    {
        return;
    }
    xfree(sample->data[0]);
    xfree(sample->data[1]);
    xfree(sample);
    return;
}


