

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
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <Generator.h>
#include <Generator_sine.h>
#include <Generator_sawtooth.h>
#include <Generator_triangle.h>
#include <Generator_square.h>
#include <Generator_square303.h>
#include <Generator_pcm.h>
#include <Generator_noise.h>
#include <File_base.h>
#include <File_tree.h>
#include <Filter.h>
#include <Event_ins.h>

#include <xmemory.h>


bool Generator_init(Generator* gen)
{
    assert(gen != NULL);
    gen->enabled = true;
    gen->volume_dB = 0;
    gen->volume = 1;
    return true;
}


Generator* new_Generator_from_file_tree(File_tree* tree,
                                        Read_state* state,
                                        Instrument_params* ins_params)
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
        Read_state_set_error(state, "Generator is not a directory");
        return NULL;
    }
    File_tree* type_tree = File_tree_get_child(tree, "gen_type.json");
    if (type_tree == NULL)
    {
        Read_state_set_error(state, "Generator does not contain a type description");
        return NULL;
    }
    Read_state_init(state, File_tree_get_path(type_tree));
    if (File_tree_is_dir(type_tree))
    {
        Read_state_set_error(state, "Type description of the Generator is a directory");
        return NULL;
    }
    char* str = File_tree_get_data(type_tree);
    char type_str[128] = { '\0' };
    str = read_string(str, type_str, 128, state);
    if (state->error)
    {
        return NULL;
    }
    Generator* gen = NULL;
    if (strcmp(type_str, "sine") == 0)
    {
        gen = (Generator*)new_Generator_sine(ins_params);
    }
    else if (strcmp(type_str, "sawtooth") == 0)
    {
        gen = (Generator*)new_Generator_sawtooth(ins_params);
    }
    else if (strcmp(type_str, "triangle") == 0)
    {
        gen = (Generator*)new_Generator_triangle(ins_params);
    }
    else if (strcmp(type_str, "square") == 0)
    {
        gen = (Generator*)new_Generator_square(ins_params);
    }
    else if (strcmp(type_str, "square303") == 0)
    {
        gen = (Generator*)new_Generator_square303(ins_params);
    }
    else if (strcmp(type_str, "pcm") == 0)
    {
        gen = (Generator*)new_Generator_pcm(ins_params);
    }
    else if (strcmp(type_str, "noise") == 0)
    {
        gen = (Generator*)new_Generator_noise(ins_params);
    }
    else
    {
        Read_state_set_error(state, "Unsupported Generator type: %s", type_str);
        return NULL;
    }
    if (gen == NULL)
    {
        Read_state_set_error(state, "Couldn't allocate memory for the new Generator");
        return NULL;
    }

    File_tree* info_tree = File_tree_get_child(tree, "generator.json");
    if (info_tree != NULL)
    {
        Read_state_init(state, File_tree_get_path(info_tree));
        if (File_tree_is_dir(info_tree))
        {
            del_Generator(gen);
            Read_state_set_error(state,
                     "Field description of the Generator is a directory");
            return NULL;
        }
        str = File_tree_get_data(info_tree);
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            del_Generator(gen);
            return NULL;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            char key[128] = { '\0' };
            bool expect_key = true;
            while (expect_key)
            {
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    del_Generator(gen);
                    return NULL;
                }
                if (strcmp(key, "enabled") == 0)
                {
                    str = read_bool(str, &gen->enabled, state);
                }
                else if (strcmp(key, "volume") == 0)
                {
                    str = read_double(str, &gen->volume_dB, state);
                    if (!state->error)
                    {
                        gen->volume = exp2(gen->volume_dB / 6);
                    }
                }
                else
                {
                    del_Generator(gen);
                    Read_state_set_error(state,
                             "Unsupported key in Generator info: %s", key);
                    return NULL;
                }
                if (state->error)
                {
                    del_Generator(gen);
                    return NULL;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                del_Generator(gen);
                return NULL;
            }
        }
    }

    assert(gen->read != NULL);
    gen->read(gen, tree, state);
    if (state->error)
    {
        del_Generator(gen);
        return NULL;
    }
    return gen;
}


Gen_type Generator_get_type(Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}


void Generator_process_note(Generator* gen,
                            Voice_state* state,
                            int note,
                            int mod,
                            int octave)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(note >= 0);
    assert(note < KQT_SCALE_NOTES);
    assert(mod < KQT_SCALE_NOTE_MODS);
    assert(octave >= KQT_SCALE_OCTAVE_FIRST);
    assert(octave <= KQT_SCALE_OCTAVE_LAST);
    if (gen->ins_params->scale == NULL ||
            *gen->ins_params->scale == NULL ||
            **gen->ins_params->scale == NULL)
    {
        return;
    }
    pitch_t pitch = Scale_get_pitch(**gen->ins_params->scale, note, mod, octave);
    if (pitch > 0)
    {
        state->pitch = pitch;
    }
    return;
}


#if 0
bool Generator_add_event(Generator* gen, Event* event, uint32_t pos)
{
    assert(gen != NULL);
    assert(event != NULL);
    return Event_queue_ins(gen->events, event, pos);
}
#endif


void Generator_mix(Generator* gen,
                   Voice_state* state,
                   uint32_t nframes,
                   uint32_t offset,
                   uint32_t freq,
                   double tempo)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    uint32_t mixed = offset;
    while (mixed < nframes)
    {
        kqt_frame** bufs = gen->ins_params->bufs;
        if ((state->filter_update && state->filter_xfade_pos >= 1)
                || freq != state->freq)
        {
            state->filter_xfade_state_used = state->filter_state_used;
            state->filter_xfade_pos = 0;
            state->filter_xfade_update = 200.0 / freq;
            if (state->actual_filter < freq / 2)
            {
                int new_state = (state->filter_state_used + 1) % 2;
                bilinear_butterworth_lowpass_filter_create(FILTER_ORDER,
                        state->actual_filter / freq,
                        state->filter_resonance,
                        state->filter_state[new_state].coeffs1,
                        state->filter_state[new_state].coeffs2);
                for (int i = 0; i < gen->ins_params->buf_count; ++i)
                {
                    for (int k = 0; k < FILTER_ORDER; ++k)
                    {
                        state->filter_state[new_state].history1[i][k] = 0;
                        state->filter_state[new_state].history2[i][k] = 0;
                    }
                }
                state->filter_state_used = new_state;
            }
            else
            {
                if (state->filter_state_used == -1)
                {
                    state->filter_xfade_pos = 1;
                }
                state->filter_state_used = -1;
            }
            state->effective_filter = state->actual_filter;
            state->effective_resonance = state->filter_resonance;
            state->filter_update = false;
        }

        uint32_t mix_until = nframes;
#if 0
        Event* ins_event = NULL;
        uint32_t ins_event_pos = 0;
        if (Event_queue_peek(gen->events, 0, &ins_event, &ins_event_pos))
        {
            if (ins_event_pos < mix_until)
            {
                mix_until = ins_event_pos;
            }
            else
            {
                ins_event = NULL;
            }
        }
#endif

        if (state->filter_state_used > -1 || state->filter_xfade_state_used > -1)
        {
            bufs = gen->ins_params->vbufs;
            if (state->filter_xfade_pos < 1 && mix_until - offset >
                    (1 - state->filter_xfade_pos) / state->filter_xfade_update)
            {
                mix_until = offset +
                        ceil((1 - state->filter_xfade_pos) / state->filter_xfade_update);
            }
        }

        mixed = gen->mix(gen, state, mix_until, mixed, freq, tempo,
                         gen->ins_params->buf_count,
                         bufs);

#if 0
        if (ins_event != NULL && mixed == mix_until)
        {
            fprintf(stderr, "Instrument event at %d! \n", (int)ins_event_pos);
            Event_queue_get(gen->events, &ins_event, &ins_event_pos);
            assert(ins_event_pos == mix_until);
            assert(ins_event != NULL);
            assert(EVENT_IS_INS(Event_get_type(ins_event)));
            Event_ins_process((Event_ins*)ins_event, gen->ins_params);
        }
#endif

        if (bufs == gen->ins_params->vbufs)
        {
            assert(state->filter_state_used != state->filter_xfade_state_used);
            kqt_frame** in_buf = gen->ins_params->vbufs;
            if (state->filter_state_used > -1)
            {
                in_buf = gen->ins_params->vbufs2;
                for (int i = 0; i < gen->ins_params->buf_count; ++i)
                {
                    iir_filter_df1_old(FILTER_ORDER, FILTER_ORDER,
                                   state->filter_state[state->filter_state_used].coeffs1,
                                   state->filter_state[state->filter_state_used].coeffs2,
                                   state->filter_state[state->filter_state_used].history1[i],
                                   state->filter_state[state->filter_state_used].history2[i],
                                   mixed - offset,
                                   gen->ins_params->vbufs[i] + offset,
                                   gen->ins_params->vbufs2[i] + offset);
                }
            }
            double vol = state->filter_xfade_pos;
            for (uint32_t k = offset; k < mixed; ++k)
            {
                if (vol > 1)
                {
                    vol = 1;
                }
                for (int i = 0; i < gen->ins_params->buf_count; ++i)
                {
                    gen->ins_params->bufs[i][k] += in_buf[i][k] * vol;
                }
                vol += state->filter_xfade_update;
            }
            if (state->filter_xfade_pos < 1)
            {
                kqt_frame** fade_buf = gen->ins_params->vbufs;
                if (state->filter_xfade_state_used > -1)
                {
                    fade_buf = gen->ins_params->vbufs2;
                    for (int i = 0; i < gen->ins_params->buf_count; ++i)
                    {
                        for (uint32_t k = 0; k < nframes; ++k)
                        {
                            gen->ins_params->vbufs2[i][k] = 0;
                        }
                    }
                    for (int i = 0; i < gen->ins_params->buf_count; ++i)
                    {
                        iir_filter_df1_old(FILTER_ORDER, FILTER_ORDER,
                                state->filter_state[state->filter_xfade_state_used].coeffs1,
                                state->filter_state[state->filter_xfade_state_used].coeffs2,
                                state->filter_state[state->filter_xfade_state_used].history1[i],
                                state->filter_state[state->filter_xfade_state_used].history2[i],
                                mixed - offset,
                                gen->ins_params->vbufs[i] + offset,
                                gen->ins_params->vbufs2[i] + offset);
                    }
                }
                double vol = 1 - state->filter_xfade_pos;
                for (uint32_t k = offset; k < mixed; ++k)
                {
                    if (vol <= 0)
                    {
                        break;
                    }
                    for (int i = 0; i < gen->ins_params->buf_count; ++i)
                    {
                        gen->ins_params->bufs[i][k] += fade_buf[i][k] * vol;
                    }
                    vol -= state->filter_xfade_update;
                }
            }
            for (int i = 0; i < gen->ins_params->buf_count; ++i)
            {
                for (uint32_t k = 0; k < nframes; ++k)
                {
                    gen->ins_params->vbufs[i][k] = 0;
                    gen->ins_params->vbufs2[i][k] = 0;
                }
            }
            state->filter_xfade_pos += state->filter_xfade_update * (mixed - offset);
        }
        offset = mixed;
        if (!state->active)
        {
            break;
        }
    }
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    gen->destroy(gen);
    return;
}


