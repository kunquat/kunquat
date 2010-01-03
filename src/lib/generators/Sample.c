

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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
#include <kunquat/limits.h>
#include <math_common.h>

#include <xmemory.h>


Sample_params* Sample_params_init(Sample_params* params)
{
    assert(params != NULL);
    params->format = SAMPLE_FORMAT_NONE;
    params->mid_freq = 48000;
    params->loop = SAMPLE_LOOP_OFF;
    params->loop_start = 0;
    params->loop_end = 0;
    return params;
}


void Sample_set_params(Sample* sample, Sample_params* params)
{
    assert(sample != NULL);
    assert(params != NULL);
    sample->params.mid_freq = params->mid_freq;
    sample->params.loop = params->loop;
    sample->params.loop_start = params->loop_start;
    sample->params.loop_end = params->loop_end;
    return;
}


Sample* new_Sample(void)
{
    Sample* sample = xalloc(Sample);
    if (sample == NULL)
    {
        return NULL;
    }
    Sample_params_init(&sample->params);
    sample->path = NULL;
    sample->changed = false;
    sample->is_lossy = false;
    sample->channels = 1;
    sample->bits = 16;
    sample->is_float = false;
    sample->len = 0;
    sample->data[0] = NULL;
    sample->data[1] = NULL;
    return sample;
}


Sample* new_Sample_from_file_tree(File_tree* tree, Read_state* state)
{
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Sample description is not a directory");
        return NULL;
    }
    File_tree* info_tree = File_tree_get_child(tree, "p_sample.json");
    if (info_tree == NULL)
    {
        return NULL;
    }
    Read_state_init(state, File_tree_get_path(info_tree));
    if (File_tree_is_dir(info_tree))
    {
        Read_state_set_error(state, "Sample description is a directory");
        return NULL;
    }
    char* str = File_tree_get_data(info_tree);
    str = read_const_char(str, '{', state);
    if (state->error)
    {
        return NULL;
    }
    str = read_const_char(str, '}', state);
    if (!state->error)
    {
        return NULL;
    }
    Read_state_clear_error(state);
    Sample_loop loop = SAMPLE_LOOP_OFF;
    int64_t loop_start = 0;
    int64_t loop_end = 0;
    double mid_freq = 440;
    Sample* sample = NULL;
    bool expect_key = true;
    while (expect_key)
    {
        char key[128] = { '\0' };
        str = read_string(str, key, 128, state);
        str = read_const_char(str, ':', state);
        if (state->error)
        {
            del_Sample(sample);
            return NULL;
        }
        if (strcmp(key, "format") == 0)
        {
            char format[32] = { '\0' };
            str = read_string(str, format, 32, state);
            if (!state->error)
            {
                if (strcmp(format, "WavPack") == 0)
                {
                    File_tree* sample_tree = File_tree_get_child(tree, "p_sample.wv");
                    if (sample_tree == NULL)
                    {
                        Read_state_set_error(state, "Sample format is set to WavPack"
                                                    " but p_sample.wv couldn't be found");
                    }
                    sample = File_tree_remove_sample(sample_tree);
                    assert(sample != NULL);
                }
/*                else if (strcmp(format, "Ogg Vorbis") == 0)
                {
                } */
                else
                {
                    Read_state_set_error(state, "Unrecognised Sample format: %s", format);
                }
            }
        }
        else if (strcmp(key, "mid_freq") == 0)
        {
            str = read_double(str, &mid_freq, state);
            if (!(sample->params.mid_freq > 0))
            {
                Read_state_set_error(state, "Sample frequency is not positive");
            }
        }
        else if (strcmp(key, "loop_mode") == 0)
        {
            char mode[] = "off!";
            str = read_string(str, mode, 5, state);
            if (strcmp(mode, "off") == 0)
            {
                loop = SAMPLE_LOOP_OFF;
            }
            else if (strcmp(mode, "uni") == 0)
            {
                loop = SAMPLE_LOOP_UNI;
            }
            else if (strcmp(mode, "bi") == 0)
            {
                loop = SAMPLE_LOOP_BI;
            }
            else
            {
                Read_state_set_error(state, "Invalid Sample loop mode (must be"
                                            " \"off\", \"uni\" or \"bi\")");
            }
        }
        else if (strcmp(key, "loop_start") == 0)
        {
            str = read_int(str, &loop_start, state);
        }
        else if (strcmp(key, "loop_end") == 0)
        {
            str = read_int(str, &loop_end, state);
        }
        else
        {
            Read_state_set_error(state, "Unsupported key in Sample header: %s", key);
        }
        if (state->error)
        {
            if (sample != NULL)
            {
                del_Sample(sample);
            }
            return NULL;
        }
        check_next(str, state, expect_key);
    }
    str = read_const_char(str, '}', state);
    if (sample == NULL)
    {
        return NULL;
    }
    if (state->error)
    {
        del_Sample(sample);
        return NULL;
    }
    sample->params.mid_freq = mid_freq;
    Sample_set_loop(sample, loop);
    Sample_set_loop_end(sample, loop_end);
    Sample_set_loop_start(sample, loop_start);
    return sample;
}


Sample_format Sample_get_format(Sample* sample)
{
    assert(sample != NULL);
    return sample->params.format;
}


char* Sample_get_path(Sample* sample)
{
    assert(sample != NULL);
    return sample->path;
}


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
    if (end <= sample->params.loop_start || end > sample->params.loop_end)
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
                    int buf_count,
                    kqt_frame** bufs,
                    double middle_tone,
                    double middle_freq)
{
    assert(sample != NULL);
    assert(gen != NULL);
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    assert(buf_count > 0);
    (void)buf_count;
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
    uint32_t mixed = offset;
    for (; mixed < nframes; ++mixed)
    {
        if (state->rel_pos >= sample->len)
        {
            state->active = false;
            break;
        }
        
        Generator_common_handle_filter(gen, state);
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
        kqt_frame vals[KQT_BUFFERS_MAX] = { 0 };
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
            vals[0] = vals[1] = ((kqt_frame)cur / 0x80) * (1 - mix_factor)
                    + ((kqt_frame)next / 0x80) * mix_factor;
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
            vals[0] = vals[1] = ((kqt_frame)cur / 0x8000) * (1 - mix_factor)
                    + ((kqt_frame)next / 0x8000) * mix_factor;
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
            vals[0] = vals[1] = ((kqt_frame)cur / 0x80000000UL) * (1 - mix_factor)
                    + ((kqt_frame)next / 0x80000000UL) * mix_factor;
        }
        Generator_common_handle_force(gen, state, vals, 2);
        double advance = (state->actual_pitch / middle_tone) * middle_freq / freq;
        uint64_t adv = floor(advance);
        double adv_rem = advance - adv;
        state->pos += adv;
        state->pos_rem += adv_rem;
        Generator_common_handle_note_off(gen, state, vals, 2, freq);
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0];
        bufs[1][mixed] += vals[1];
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
    Generator_common_persist(gen, state, mixed);
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


