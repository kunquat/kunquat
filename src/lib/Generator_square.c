

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
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <Generator.h>
#include <Generator_common.h>
#include <Generator_square.h>
#include <Voice_state_square.h>
#include <Song_limits.h>

#include <xmemory.h>


static bool Generator_square_read(Generator* gen, File_tree* tree, Read_state* state);

void Generator_square_init_state(Generator* gen, Voice_state* state);


Generator_square* new_Generator_square(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_square* square = xalloc(Generator_square);
    if (square == NULL)
    {
        return NULL;
    }
    Generator_init(&square->parent);
    square->parent.read = Generator_square_read;
    square->parent.destroy = del_Generator_square;
    square->parent.type = GEN_TYPE_SQUARE;
    square->parent.init_state = Generator_square_init_state;
    square->parent.mix = Generator_square_mix;
    square->parent.ins_params = ins_params;
    square->pulse_width = 0.5;
    return square;
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
    File_tree* square_tree = File_tree_get_child(tree, "square.json");
    if (square_tree == NULL)
    {
        return true;
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


void Generator_square_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    assert(state != NULL);
    Voice_state_init(state);
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
                              int buf_count,
                              frame_t** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(buf_count > 0);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Generator_common_check_active(gen, state, offset);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_square* square_state = (Voice_state_square*)state;
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double vals[BUF_COUNT_MAX] = { 0 };
        vals[0] = vals[1] = square(square_state->phase, square_state->pulse_width) / 6;
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
        square_state->phase += state->freq / freq;
        if (square_state->phase >= 1)
        {
            square_state->phase -= floor(square_state->phase);
        }
        state->pos = 1; // XXX: hackish
        Generator_common_handle_note_off(gen, state, vals, 2, freq, i);
        bufs[0][i] += vals[0];
        bufs[1][i] += vals[1];
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return nframes;
}


void del_Generator_square(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE);
    Generator_square* square = (Generator_square*)gen;
    xfree(square);
    return;
}


