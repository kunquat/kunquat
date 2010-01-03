

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
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include <Generator.h>
#include <Generator_common.h>
#include <Generator_square.h>
#include <Voice_state_square.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static bool Generator_square_read(Generator* gen, File_tree* tree, Read_state* state);

static void Generator_square_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_square(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_square* square = xalloc(Generator_square);
    if (square == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&square->parent))
    {
        xfree(square);
        return NULL;
    }
    square->parent.read = Generator_square_read;
    square->parent.destroy = del_Generator_square;
    square->parent.type = GEN_TYPE_SQUARE;
    square->parent.init_state = Generator_square_init_state;
    square->parent.mix = Generator_square_mix;
    square->parent.ins_params = ins_params;
    square->pulse_width = 0.5;
    return &square->parent;
}


bool Generator_square_has_subkey(const char* subkey)
{
    assert(subkey != NULL);
    return strcmp(subkey, "gen_square/p_square.json") == 0;
}


bool Generator_square_parse(Generator* gen,
                            const char* subkey,
                            void* data,
                            long length,
                            Read_state* state)
{
    assert(gen != NULL);
    assert(Generator_get_type(gen) == GEN_TYPE_SQUARE);
    assert(subkey != NULL);
    assert(Generator_square_has_subkey(subkey));
    assert(data != NULL || length == 0);
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Generator_square* gen_square = (Generator_square*)gen;
    if (strcmp(subkey, "gen_square/p_square.json") == 0)
    {
        double pulse_width = 0.5;
        char* str = data;
        if (str != NULL)
        {
            str = read_const_char(str, '{', state);
            str = read_const_string(str, "pulse_width", state);
            str = read_const_char(str, ':', state);
            str = read_double(str, &pulse_width, state);
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
        gen_square->pulse_width = pulse_width;
        return true;
    }
    return false;
}


static bool Generator_square_read(Generator* gen, File_tree* tree, Read_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    assert(tree != NULL);
    assert(File_tree_is_dir(tree));
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    File_tree* dir_tree = File_tree_get_child(tree, "gen_square");
    if (dir_tree == NULL)
    {
        return true;
    }
    if (!File_tree_is_dir(dir_tree))
    {
        Read_state_set_error(state, "Square Generator is not a directory");
        return false;
    }
    File_tree* square_tree = File_tree_get_child(dir_tree, "p_square.json");
    if (square_tree == NULL)
    {
        return true;
    }
    if (File_tree_is_dir(square_tree))
    {
        Read_state_set_error(state, "Square Generator description is a directory");
        return false;
    }
    Generator_square* square = (Generator_square*)gen;
    char* str = File_tree_get_data(square_tree);
    str = read_const_char(str, '{', state);
    str = read_const_string(str, "pulse_width", state);
    str = read_const_char(str, ':', state);
    str = read_double(str, &square->pulse_width, state);
    str = read_const_char(str, '}', state);
    if (state->error)
    {
        return false;
    }
    return true;
}


static void Generator_square_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    assert(state != NULL);
    Voice_state_square* square_state = (Voice_state_square*)state;
    Generator_square* square = (Generator_square*)gen;
    square_state->phase = 0;
    square_state->pulse_width = square->pulse_width;
    return;
}


double square(double phase, double pulse_width)
{
    if (phase < pulse_width)
    {
        return 1.0;
    }
    return -1.0;
}


uint32_t Generator_square_mix(Generator* gen,
                              Voice_state* state,
                              uint32_t nframes,
                              uint32_t offset,
                              uint32_t freq,
                              double tempo,
                              int buf_count,
                              kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(tempo > 0);
    assert(buf_count > 0);
    (void)buf_count;
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_square* square_state = (Voice_state_square*)state;
    uint32_t mixed = offset;
    for (; mixed < nframes; ++mixed)
    {
        Generator_common_handle_filter(gen, state);
        Generator_common_handle_pitch(gen, state);

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = square(square_state->phase, square_state->pulse_width) / 6;
        Generator_common_handle_force(gen, state, vals, 1);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
        square_state->phase += state->actual_pitch / freq;
        if (square_state->phase >= 1)
        {
            square_state->phase -= floor(square_state->phase);
        }
        state->pos = 1; // XXX: hackish
        Generator_common_handle_note_off(gen, state, vals, 1, freq);
        vals[1] = vals[0];
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0];
        bufs[1][mixed] += vals[1];
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    Generator_common_persist(gen, state, mixed);
    return mixed;
}


void del_Generator_square(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    Generator_square* square = (Generator_square*)gen;
    xfree(square);
    return;
}


