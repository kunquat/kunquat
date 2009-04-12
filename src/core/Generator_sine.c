

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
#include <Generator_sine.h>

#include <xmemory.h>


#define PI_2 (3.14159265358979323846 * 2)


Generator_sine* new_Generator_sine(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_sine* sine = xalloc(Generator_sine);
    if (sine == NULL)
    {
        return NULL;
    }
    sine->parent.destroy = del_Generator_sine;
    sine->parent.type = GEN_TYPE_SINE;
    sine->parent.init_state = Voice_state_sine_init;
    sine->parent.mix = Generator_sine_mix;
    sine->parent.ins_params = ins_params;
    return sine;
}


void Generator_sine_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(gen->ins_params->bufs[0] != NULL);
    assert(gen->ins_params->bufs[1] != NULL);
    if (!state->active)
    {
        return;
    }
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double val_l = 0;
        double val_r = 0;
        val_l = val_r = sin(state->ins_fields.sine.phase) / 6;
        if (state->pos_rem < 0.002)
        {
            val_l = val_r = val_l * (state->pos_rem * 500);
            state->pos_rem += 1.0 / freq;
        }
        if (!state->note_on)
        {
            if (state->noff_pos_rem < 0.002)
            {
                val_l = val_r = val_l * (1 - (state->noff_pos_rem * 333));
            }
            else
            {
                val_l = val_r = (val_l / 3) * (1 - state->noff_pos_rem);
            }
        }
        state->ins_fields.sine.phase += state->freq * PI_2 / freq;
        if (state->ins_fields.sine.phase >= PI_2)
        {
            state->ins_fields.sine.phase -= floor(state->ins_fields.sine.phase / PI_2) * PI_2;
        }
        gen->ins_params->bufs[0][i] += val_l;
        gen->ins_params->bufs[1][i] += val_r;
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
        if (!state->note_on)
        {
            state->noff_pos_rem += 1.0 / freq;
            if (state->noff_pos_rem >= 1)
            {
                state->active = false;
                return;
            }
        }
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return;
}


void del_Generator_sine(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
    Generator_sine* sine = (Generator_sine*)gen;
    xfree(sine);
    return;
}


