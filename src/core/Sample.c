

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
#include <File_wavpack.h>

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
    if (!state->active)
    {
        return;
    }
    for (uint32_t i = offset; i < nframes; ++i)
    {
        if (state->rel_pos >= sample->len)
        {
            state->active = false;
            break;
        }
        frame_t val_l = 0;
        frame_t val_r = 0;
        if (sample->is_float)
        {
            float* buf_l = sample->data[0];
//          float* buf_r = sample->data[1];
            float cur = buf_l[state->rel_pos];
            float next = 0;
            if (state->rel_pos + 1 < sample->len)
            {
                next = buf_l[state->rel_pos + 1];
            }
            val_l = val_r = cur * (1 - state->rel_pos_rem)
                    + next * (state->rel_pos_rem);
        }
        else if (sample->bits == 8)
        {
            int8_t* buf_l = sample->data[0];
//          int8_t* buf_r = sample->data[1];
            int8_t cur = buf_l[state->rel_pos];
            int8_t next = 0;
            if (state->rel_pos + 1 < sample->len)
            {
                next = buf_l[state->rel_pos + 1];
            }
            val_l = val_r = ((frame_t)cur / 0x80) * (1 - state->rel_pos_rem)
                    + ((frame_t)next / 0x80) * (state->rel_pos_rem);
        }
        else if (sample->bits == 16)
        {
            int16_t* buf_l = sample->data[0];
//          int16_t* buf_r = sample->data[1];
            int16_t cur = buf_l[state->rel_pos];
            int16_t next = 0;
            if (state->rel_pos + 1 < sample->len)
            {
                next = buf_l[state->rel_pos + 1];
            }
            val_l = val_r = ((frame_t)cur / 0x8000) * (1 - state->rel_pos_rem)
                    + ((frame_t)next / 0x8000) * (state->rel_pos_rem);
        }
        else
        {
            assert(sample->bits == 32);
            int16_t* buf_l = sample->data[0];
//          int16_t* buf_r = sample->data[1];
            int16_t cur = buf_l[state->rel_pos];
            int16_t next = 0;
            if (state->rel_pos + 1 < sample->len)
            {
                next = buf_l[state->rel_pos + 1];
            }
            val_l = val_r = ((frame_t)cur / 0x80000000UL) * (1 - state->rel_pos_rem)
                    + ((frame_t)next / 0x80000000UL) * (state->rel_pos_rem);
        }
        if (!state->note_on && gen->ins_params->volume_off_env_enabled)
        {
            double scale = Envelope_get_value(gen->ins_params->volume_off_env,
                    state->off_ve_pos);
            if (!isfinite(scale))
            {
                state->active = false;
                break;
            }
            if (state->pedal < 0.5)
            {
                state->off_ve_pos += 1.0 / freq;
            }
            val_l *= scale;
            val_r *= scale;
        }
        gen->ins_params->bufs[0][i] += val_l;
        gen->ins_params->bufs[1][i] += val_r;
        double advance = (state->freq / 440) * sample->mid_freq / freq;
        uint64_t adv = floor(advance);
        double adv_rem = advance - adv;
        state->rel_pos += adv;
        state->rel_pos_rem += adv_rem;
        if (state->rel_pos_rem >= 1)
        {
            state->rel_pos_rem -= 1;
            ++state->rel_pos;
        }
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


