

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <Sample.h>
#include <Generator_common.h>
#include <File_wavpack.h>
#include <Song_limits.h>
#include <math_common.h>

#include <xmemory.h>


Sample* new_Sample(void)
{
    Sample* sample = xalloc(Sample);
    if (sample == NULL)
    {
        return NULL;
    }
    sample->path = NULL;
    sample->format = SAMPLE_FORMAT_NONE;
    sample->changed = false;
    sample->is_lossy = false;
    sample->channels = 1;
    sample->bits = 16;
    sample->is_float = false;
    sample->loop = SAMPLE_LOOP_OFF;
    sample->loop_start = 0;
    sample->loop_end = 0;
    sample->len = 0;
    sample->mid_freq = 48000;
    sample->data[0] = NULL;
    sample->data[1] = NULL;
    return sample;
}


bool Sample_load(Sample* sample, FILE* in, Sample_format format)
{
    assert(sample != NULL);
    assert(in != NULL);
    assert(format > SAMPLE_FORMAT_NONE);
    assert(format < SAMPLE_FORMAT_LAST);
    for (int i = 0; i < 2; ++i)
    {
        if (sample->data[i] != NULL)
        {
            xfree(sample->data[i]);
            sample->data[i] = NULL;
        }
    }
    if (format == SAMPLE_FORMAT_WAVPACK)
    {
        return File_wavpack_load_sample(sample, in);
    }
    return false;
}


bool Sample_load_path(Sample* sample, char* path, Sample_format format)
{
    assert(sample != NULL);
    assert(path != NULL);
    assert(format > SAMPLE_FORMAT_NONE);
    assert(format < SAMPLE_FORMAT_LAST);
    FILE* in = fopen(path, "r");
    if (in == NULL)
    {
        return false;
    }
    bool ret = Sample_load(sample, in, format);
    fclose(in);
    if (ret)
    {
        if (sample->path != NULL)
        {
            xfree(sample->path);
            sample->path = NULL;
        }
        sample->path = xnalloc(char, strlen(path) + 1);
        if (sample->path != NULL)
        {
            strcpy(sample->path, path);
        }
    }
    return ret;
}


char* Sample_get_path(Sample* sample)
{
    assert(sample != NULL);
    return sample->path;
}


void Sample_set_loop(Sample* sample, Sample_loop loop)
{
    assert(sample != NULL);
    assert(   loop == SAMPLE_LOOP_OFF
           || loop == SAMPLE_LOOP_UNI
           || loop == SAMPLE_LOOP_BI);
    if (sample->len == 0)
    {
        sample->loop = SAMPLE_LOOP_OFF;
        return;
    }
    if (sample->loop_start > 0 && sample->loop_start >= sample->loop_end)
    {
        sample->loop_start = sample->loop_end - 1;
    }
    if (sample->loop_start >= sample->len)
    {
        sample->loop_start = sample->len - 1;
    }
    if (sample->loop_end <= sample->loop_start)
    {
        if (sample->loop_start == 0)
        {
            sample->loop_end = sample->len;
        }
        else
        {
            sample->loop_end = sample->loop_start + 1;
        }
    }
    assert(sample->loop_start < sample->loop_end);
    assert(sample->loop_end <= sample->len);
    sample->loop = loop;
    return;
}


Sample_loop Sample_get_loop(Sample* sample)
{
    assert(sample != NULL);
    return sample->loop;
}


void Sample_set_loop_start(Sample* sample, uint64_t start)
{
    assert(sample != NULL);
    if (start >= sample->len || start >= sample->loop_end)
    {
        sample->loop = SAMPLE_LOOP_OFF;
    }
    sample->loop_start = start;
    return;
}


uint64_t Sample_get_loop_start(Sample* sample)
{
    assert(sample != NULL);
    return sample->loop_start;
}


void Sample_set_loop_end(Sample* sample, uint64_t end)
{
    assert(sample != NULL);
    if (end <= sample->loop_start || end > sample->loop_end)
    {
        sample->loop = SAMPLE_LOOP_OFF;
    }
    sample->loop_end = end;
    return;
}


uint64_t Sample_get_loop_end(Sample* sample)
{
    assert(sample != NULL);
    return sample->loop_end;
}


void Sample_mix(Sample* sample,
        Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(sample != NULL);
    assert(gen != NULL);
    assert(gen->ins_params->bufs != NULL);
    assert(gen->ins_params->bufs[0] != NULL);
    assert(gen->ins_params->bufs[1] != NULL);
    assert(state != NULL);
    assert(freq > 0);
    Generator_common_check_active(gen, state);
    for (uint32_t i = offset; i < nframes; ++i)
    {
        if (state->rel_pos >= sample->len)
        {
            state->active = false;
            break;
        }
        bool next_exists = false;
        uint64_t next_pos = 0;
        if (state->dir > 0 || sample->loop != SAMPLE_LOOP_BI)
        {
            uint64_t limit = sample->len;
            if (sample->loop)
            {
                limit = sample->loop_end;
            }
            if (state->rel_pos + 1 < limit)
            {
                next_exists = true;
                next_pos = state->rel_pos + 1;
            }
            else
            {
                if (sample->loop == SAMPLE_LOOP_UNI)
                {
                    next_exists = true;
                    next_pos = sample->loop_start;
                }
                else if (sample->loop == SAMPLE_LOOP_BI)
                {
                    next_exists = true;
                    if (state->rel_pos > sample->loop_start)
                    {
                        next_pos = state->rel_pos - 1;
                    }
                    else
                    {
                        next_pos = sample->loop_start;
                    }
                }
            }
        }
        else if (sample->loop_start + 1 == sample->loop_end)
        {
            next_exists = true;
            next_pos = sample->loop_start;
        }
        else
        {
            assert(sample->loop == SAMPLE_LOOP_BI);
            next_exists = true;
            if (state->rel_pos > sample->loop_start)
            {
                next_pos = state->rel_pos - 1;
            }
            else
            {
                next_pos = sample->loop_start + 1;
            }
            if (next_pos >= sample->loop_end)
            {
                next_pos = sample->loop_end - 1;
            }
        }
        assert(!sample->loop || next_pos < sample->loop_end);
        assert(next_pos < sample->len);
        double mix_factor = state->rel_pos_rem;
        if (next_pos < state->rel_pos)
        {
            mix_factor = 1 - state->rel_pos_rem;
        }
        frame_t vals[BUF_COUNT_MAX] = { 0 };
        if (sample->is_float)
        {
            float* buf_l = sample->data[0];
//          float* buf_r = sample->data[1];
            float cur = buf_l[state->rel_pos];
            float next = 0;
            if (next_exists)
            {
                next = buf_l[next_pos];
            }
            vals[0] = vals[1] = cur * (1 - mix_factor)
                    + next * mix_factor;
        }
        else if (sample->bits == 8)
        {
            int8_t* buf_l = sample->data[0];
//          int8_t* buf_r = sample->data[1];
            int8_t cur = buf_l[state->rel_pos];
            int8_t next = 0;
            if (next_exists)
            {
                next = buf_l[next_pos];
            }
            vals[0] = vals[1] = ((frame_t)cur / 0x80) * (1 - mix_factor)
                    + ((frame_t)next / 0x80) * mix_factor;
        }
        else if (sample->bits == 16)
        {
            int16_t* buf_l = sample->data[0];
//          int16_t* buf_r = sample->data[1];
            int16_t cur = buf_l[state->rel_pos];
            int16_t next = 0;
            if (next_exists)
            {
                next = buf_l[next_pos];
            }
            vals[0] = vals[1] = ((frame_t)cur / 0x8000) * (1 - mix_factor)
                    + ((frame_t)next / 0x8000) * mix_factor;
        }
        else
        {
            assert(sample->bits == 32);
            int16_t* buf_l = sample->data[0];
//          int16_t* buf_r = sample->data[1];
            int16_t cur = buf_l[state->rel_pos];
            int16_t next = 0;
            if (next_exists)
            {
                next = buf_l[next_pos];
            }
            vals[0] = vals[1] = ((frame_t)cur / 0x80000000UL) * (1 - mix_factor)
                    + ((frame_t)next / 0x80000000UL) * mix_factor;
        }
        Generator_common_handle_note_off(gen, state, vals, 2, freq);
        gen->ins_params->bufs[0][i] += vals[0];
        gen->ins_params->bufs[1][i] += vals[1];
        double advance = (state->freq / 440) * sample->mid_freq / freq;
        uint64_t adv = floor(advance);
        double adv_rem = advance - adv;
        state->pos += adv;
        state->pos_rem += adv_rem;
        if (state->pos_rem >= 1)
        {
            state->pos += floor(state->pos_rem);
            state->pos_rem -= floor(state->pos_rem);
        }
        if (sample->loop == SAMPLE_LOOP_OFF)
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
            uint64_t loop_len = sample->loop_end - sample->loop_start;
            uint64_t limit = sample->loop == SAMPLE_LOOP_UNI
                                           ? loop_len
                                           : 2 * loop_len - 2;
            uint64_t virt_pos = state->pos;
            if (virt_pos < sample->loop_end)
            {
                state->dir = 1;
                state->rel_pos = state->pos;
                state->rel_pos_rem = state->pos_rem;
            }
            else if (sample->loop_start + 1 == sample->loop_end)
            {
                state->dir = 1;
                state->rel_pos = sample->loop_start;
                state->rel_pos_rem = 0;
            }
            else
            {
                virt_pos = ((virt_pos - sample->loop_start) % limit) + sample->loop_start;
                if (sample->loop == SAMPLE_LOOP_UNI || virt_pos < sample->loop_end - 1)
                {
                    assert(sample->loop != SAMPLE_LOOP_UNI || virt_pos < sample->loop_end);
                    state->dir = 1;
                    state->rel_pos = virt_pos;
                    state->rel_pos_rem = state->pos_rem;
                }
                else
                {
                    assert(sample->loop == SAMPLE_LOOP_BI);
                    assert(virt_pos >= sample->loop_end - 1);
                    state->dir = -1;
                    uint64_t back = virt_pos - (sample->loop_end - 1);
                    state->rel_pos = sample->loop_end - 1 - back;
                    state->rel_pos_rem = state->pos_rem;
                    if (state->rel_pos > sample->loop_start && state->rel_pos_rem > 0)
                    {
                        --state->rel_pos;
                        state->rel_pos_rem = 1 - state->rel_pos_rem;
                    }
                }
            }
        }
        assert(state->rel_pos < sample->len);
    }
    return;
}


void Sample_set_freq(Sample* sample, double freq)
{
    assert(sample != NULL);
    sample->mid_freq = freq;
    return;
}


double Sample_get_freq(Sample* sample)
{
    assert(sample != NULL);
    return sample->mid_freq;
}


uint64_t Sample_get_len(Sample* sample)
{
    assert(sample != NULL);
    return sample->len;
}


void del_Sample(Sample* sample)
{
    assert(sample != NULL);
    if (sample->path != NULL)
    {
        xfree(sample->path);
    }
    if (sample->data[0] != NULL)
    {
        xfree(sample->data[0]);
    }
    if (sample->data[1] != NULL)
    {
        xfree(sample->data[1]);
    }
    xfree(sample);
    return;
}


