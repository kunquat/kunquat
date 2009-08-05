

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
#include <Generator_triangle.h>
#include <Voice_state_triangle.h>
#include <kunquat/limits.h>

#include <xmemory.h>


bool Generator_triangle_read(Generator* gen, File_tree* tree, Read_state* state);

void Generator_triangle_init_state(Generator* gen, Voice_state* state);


Generator_triangle* new_Generator_triangle(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_triangle* triangle = xalloc(Generator_triangle);
    if (triangle == NULL)
    {
        return NULL;
    }
    Generator_init(&triangle->parent);
    triangle->parent.read = Generator_triangle_read;
    triangle->parent.destroy = del_Generator_triangle;
    triangle->parent.type = GEN_TYPE_TRIANGLE;
    triangle->parent.init_state = Generator_triangle_init_state;
    triangle->parent.mix = Generator_triangle_mix;
    triangle->parent.ins_params = ins_params;
    return triangle;
}


bool Generator_triangle_read(Generator* gen, File_tree* tree, Read_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_TRIANGLE);
    assert(tree != NULL);
    assert(state != NULL);
    (void)gen;
    (void)tree;
    if (state->error)
    {
        return false;
    }
    return true;
}


void Generator_triangle_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_TRIANGLE);
    (void)gen;
    assert(state != NULL);
    Voice_state_triangle* triangle_state = (Voice_state_triangle*)state;
    triangle_state->phase = 0.25;
    return;
}


double triangle(double phase)
{
    if (phase < 0.5)
    {
        return (phase * 4) - 1;
    }
    return (phase * (-4)) + 3;
}


uint32_t Generator_triangle_mix(Generator* gen,
                                Voice_state* state,
                                uint32_t nframes,
                                uint32_t offset,
                                uint32_t freq,
                                double tempo,
                                int buf_count,
                                kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_TRIANGLE);
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
    Voice_state_triangle* triangle_state = (Voice_state_triangle*)state;
    uint32_t mixed = offset;
    for (; mixed < nframes; ++mixed)
    {
        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = vals[1] = triangle(triangle_state->phase) / 6;
        Generator_common_handle_force(gen, state, vals, 2);
        Generator_common_handle_pitch(gen, state);
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
        triangle_state->phase += state->actual_pitch / freq;
        if (triangle_state->phase >= 1)
        {
            triangle_state->phase -= floor(triangle_state->phase);
        }
        state->pos = 1; // XXX: hackish
        Generator_common_handle_note_off(gen, state, vals, 2, freq);
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


void del_Generator_triangle(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_TRIANGLE);
    Generator_triangle* triangle = (Generator_triangle*)gen;
    xfree(triangle);
    return;
}


