

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
#include <Generator_square303.h>
#include <Voice_state_square303.h>
#include <Song_limits.h>

#include <xmemory.h>


Generator_square303* new_Generator_square303(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_square303* square303 = xalloc(Generator_square303);
    if (square303 == NULL)
    {
        return NULL;
    }
    square303->parent.destroy = del_Generator_square303;
    square303->parent.type = GEN_TYPE_SQUARE303;
    square303->parent.init_state = Voice_state_square303_init;
    square303->parent.mix = Generator_square303_mix;
    square303->parent.ins_params = ins_params;
    return square303;
}


double square303(double phase)
{
    double flip = 1;
    if (phase >= 0.25 && phase < 0.75)
    {
        flip = -1;
    }
    phase *= 2;
    if (phase >= 1)
    {
        phase -= 1;
    }
    return ((phase * 2) - 1) * flip;
} 


void Generator_square303_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE303);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(gen->ins_params->bufs[0] != NULL);
    assert(gen->ins_params->bufs[1] != NULL);
    Generator_common_check_active(gen, state);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_square303* square303_state = (Voice_state_square303*)state;
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double vals[BUF_COUNT_MAX] = { 0 };
        vals[0] = vals[1] = square303(square303_state->phase) / 6;
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
        square303_state->phase += state->freq / freq;
        if (square303_state->phase >= 1)
        {
            square303_state->phase -= floor(square303_state->phase);
        }
        state->pos = 1; // XXX: hackish
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


void del_Generator_square303(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE303);
    Generator_square303* square303 = (Generator_square303*)gen;
    xfree(square303);
    return;
}


