

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


#if 0
void Sample_set_params(Sample* sample, Sample_params* params)
{
    assert(sample != NULL);
    assert(params != NULL);

    Sample_params_copy(&sample->params, params);

    return;
}
#endif


#if 0
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
        sample->params.loop_start = sample->params.loop_end - 1;

    if (sample->params.loop_start >= sample->len)
        sample->params.loop_start = sample->len - 1;

    if (sample->params.loop_end <= sample->params.loop_start)
    {
        if (sample->params.loop_start == 0)
            sample->params.loop_end = sample->len;
        else
            sample->params.loop_end = sample->params.loop_start + 1;
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
        sample->params.loop = SAMPLE_LOOP_OFF;
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
        sample->params.loop = SAMPLE_LOOP_OFF;
    sample->params.loop_end = end;

    return;
}


uint64_t Sample_get_loop_end(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.loop_end;
}
#endif


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
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
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
    assert(freq > 0);
    assert(tempo > 0);
    assert(vol_scale >= 0);

    const Work_buffer* wb_actual_pitches = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    const Work_buffer* wb_actual_forces = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_FORCES);
    const float* actual_pitches = Work_buffer_get_contents_mut(wb_actual_pitches) + 1;
    const float* actual_forces = Work_buffer_get_contents_mut(wb_actual_forces) + 1;

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);
    float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
    float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

    uint64_t new_pos = vstate->pos;

    uint32_t mixed = offset;
    for (; mixed < nframes && vstate->active; ++mixed)
    {
        if (vstate->rel_pos >= sample->len)
        {
            vstate->active = false;
            break;
        }

        const float actual_pitch = actual_pitches[mixed];
        const float actual_force = actual_forces[mixed];

        bool next_exists = false;
        uint64_t next_pos = 0;
        if (vstate->dir > 0 || params->loop != SAMPLE_LOOP_BI)
        {
            uint64_t limit = sample->len;
            if (params->loop)
                limit = params->loop_end;

            if (vstate->rel_pos + 1 < limit)
            {
                next_exists = true;
                next_pos = vstate->rel_pos + 1;
            }
            else
            {
                if (params->loop == SAMPLE_LOOP_UNI)
                {
                    next_exists = true;
                    next_pos = params->loop_start;
                }
                else if (params->loop == SAMPLE_LOOP_BI)
                {
                    next_exists = true;

                    if (vstate->rel_pos > params->loop_start)
                        next_pos = vstate->rel_pos - 1;
                    else
                        next_pos = params->loop_start;
                }
            }
        }
        else if (params->loop_start + 1 == params->loop_end)
        {
            next_exists = true;
            next_pos = params->loop_start;
        }
        else
        {
            assert(params->loop == SAMPLE_LOOP_BI);

            next_exists = true;
            if (vstate->rel_pos > params->loop_start)
                next_pos = vstate->rel_pos - 1;
            else
                next_pos = params->loop_start + 1;

            if (next_pos >= params->loop_end)
                next_pos = params->loop_end - 1;
        }

        assert(!params->loop || next_pos < params->loop_end);
        assert(next_pos < sample->len);

        double mix_factor = vstate->rel_pos_rem;
        if (next_pos < vstate->rel_pos)
            mix_factor = 1 - vstate->rel_pos_rem;

        double vals[KQT_BUFFERS_MAX] = { 0 };

#define get_items(type)                                         \
        if (true)                                               \
        {                                                       \
            type* buf_l = sample->data[0];                      \
            type cur[2] = { buf_l[vstate->rel_pos] };           \
            type next[2] = { 0 };                               \
            if (next_exists)                                    \
                next[0] = buf_l[next_pos];                      \
            vals[0] = (double)cur[0] + mix_factor *             \
                      ((double)next[0] - (double)cur[0]);       \
            if (sample->channels > 1)                           \
            {                                                   \
                type* buf_r = sample->data[1];                  \
                cur[1] = buf_r[vstate->rel_pos];                \
                if (next_exists)                                \
                    next[1] = buf_r[next_pos];                  \
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

        //Generator_common_handle_force(gen, ins_state, vstate, vals, 2, freq);
        //Generator_common_handle_filter(gen, vstate, vals, 2, freq);

        audio_l[mixed] = vals[0] * actual_force * vol_scale;
        audio_r[mixed] = vals[1] * actual_force * vol_scale;

        double advance = (actual_pitch / middle_tone) * middle_freq / freq;
        uint64_t adv = floor(advance);
        double adv_rem = advance - adv;

        new_pos += adv;
        vstate->pos_rem += adv_rem;
//        Generator_common_handle_note_off(gen, vstate, vals, 2, freq);
        //Generator_common_handle_panning(gen, vstate, vals, 2);

        //bufs[0][mixed] += vals[0] * vol_scale;
        //bufs[1][mixed] += vals[1] * vol_scale;

        if (vstate->pos_rem >= 1)
        {
            new_pos += floor(vstate->pos_rem);
            vstate->pos_rem -= floor(vstate->pos_rem);
        }
        if (params->loop == SAMPLE_LOOP_OFF)
        {
            vstate->rel_pos = new_pos;
            vstate->rel_pos_rem = vstate->pos_rem;
            if (vstate->rel_pos >= sample->len)
            {
                vstate->active = false;
                break;
            }
        }
        else
        {
            uint64_t loop_len = params->loop_end - params->loop_start;
            uint64_t limit = params->loop == SAMPLE_LOOP_UNI ?
                loop_len :
                2 * loop_len - 2;

            uint64_t virt_pos = new_pos;
            if (virt_pos < params->loop_end)
            {
                vstate->dir = 1;
                vstate->rel_pos = new_pos;
                vstate->rel_pos_rem = vstate->pos_rem;
            }
            else if (params->loop_start + 1 == params->loop_end)
            {
                vstate->dir = 1;
                vstate->rel_pos = params->loop_start;
                vstate->rel_pos_rem = 0;
            }
            else
            {
                virt_pos = ((virt_pos - params->loop_start) % limit) +
                        params->loop_start;

                if (params->loop == SAMPLE_LOOP_UNI ||
                        virt_pos < params->loop_end - 1)
                {
                    assert(params->loop != SAMPLE_LOOP_UNI ||
                            virt_pos < params->loop_end);

                    vstate->dir = 1;
                    vstate->rel_pos = virt_pos;
                    vstate->rel_pos_rem = vstate->pos_rem;
                }
                else
                {
                    assert(params->loop == SAMPLE_LOOP_BI);
                    assert(virt_pos >= params->loop_end - 1);

                    vstate->dir = -1;
                    uint64_t back = virt_pos - (params->loop_end - 1);
                    vstate->rel_pos = params->loop_end - 1 - back;
                    vstate->rel_pos_rem = vstate->pos_rem;
                    if (vstate->rel_pos > params->loop_start &&
                            vstate->rel_pos_rem > 0)
                    {
                        --vstate->rel_pos;
                        vstate->rel_pos_rem = 1 - vstate->rel_pos_rem;
                    }
                }
            }
        }

        assert(vstate->rel_pos < sample->len);
    }

    const int32_t release_limit = Generator_common_ramp_release(
            gen, ins_state, vstate, wbs, 2, freq, nframes, offset);
    if (release_limit < (int32_t)nframes)
        mixed = release_limit;
    const bool ramp_release_ended = (vstate->ramp_release >= 1);

    Generator_common_handle_filter(gen, vstate, wbs, 2, freq, mixed, offset);
    Generator_common_handle_panning(gen, vstate, wbs, mixed, offset);

    vstate->pos = new_pos;

    if (ramp_release_ended)
        vstate->active = false;

    return mixed;
}


#if 0
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
#endif


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


