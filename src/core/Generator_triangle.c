

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
#include <Song_limits.h>
#include <math_common.h>

#include <xmemory.h>


Generator_triangle* new_Generator_triangle(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_triangle* triangle = xalloc(Generator_triangle);
    if (triangle == NULL)
    {
        return NULL;
    }
    triangle->parent.destroy = del_Generator_triangle;
    triangle->parent.type = GEN_TYPE_TRIANGLE;
    triangle->parent.init_state = Voice_state_triangle_init;
    triangle->parent.mix = Generator_triangle_mix;
    triangle->parent.ins_params = ins_params;
    return triangle;
}

double triangle(double x)
{
    return acos(sin(x));
}

void Generator_triangle_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_TRIANGLE);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(gen->ins_params->bufs[0] != NULL);
    assert(gen->ins_params->bufs[1] != NULL);
    Generator_common_check_active(gen, state);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_triangle* triangle_state = (Voice_state_triangle*)state;
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double vals[BUF_COUNT_MAX] = { 0 };
        vals[0] = vals[1] = triangle(triangle_state->phase) / 6;
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
        triangle_state->phase += state->freq * PI * 2 / freq;
        if (triangle_state->phase >= PI * 2)
        {
            triangle_state->phase -= floor(triangle_state->phase / PI * 2) * PI * 2;
            ++state->pos;
        }
        Generator_common_handle_note_off(gen, state, vals, 2, freq);
        gen->ins_params->bufs[0][i] += vals[0];
        gen->ins_params->bufs[1][i] += vals[1];
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return;
}


void del_Generator_triangle(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_TRIANGLE);
    Generator_triangle* triangle = (Generator_triangle*)gen;
    xfree(triangle);
    return;
}


