

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
#include <Generator_sine.h>
#include <Voice_state_sine.h>
#include <Song_limits.h>
#include <math_common.h>

#include <xmemory.h>


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


uint32_t Generator_sine_mix(Generator* gen,
                            Voice_state* state,
                            uint32_t nframes,
                            uint32_t offset,
                            uint32_t freq,
                            int buf_count,
                            frame_t** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(buf_count > 0);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Generator_common_check_active(gen, state, offset);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_sine* sine_state = (Voice_state_sine*)state;
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double vals[BUF_COUNT_MAX] = { 0 };
        vals[0] = vals[1] = sin(sine_state->phase * PI * 2) / 6;
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
        sine_state->phase += state->freq / freq;
        if (sine_state->phase >= 1)
        {
            sine_state->phase -= floor(sine_state->phase);
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


void del_Generator_sine(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
    Generator_sine* sine = (Generator_sine*)gen;
    xfree(sine);
    return;
}


