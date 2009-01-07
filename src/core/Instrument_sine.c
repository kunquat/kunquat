

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

#include "Instrument_sine.h"


#define PI_2 (3.14159265358979323846 * 2)


void Instrument_sine_mix(Instrument* ins,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(ins != NULL);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(ins->bufs[0] != NULL);
    assert(ins->bufs[1] != NULL);
    if (!state->active)
    {
        return;
    }
    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double val_l = 0;
        double val_r = 0;
        val_l = val_r = sin(state->rel_pos_rem) / 6;
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
        state->rel_pos_rem += state->freq * PI_2 / freq;
        if (state->rel_pos_rem >= PI_2)
        {
            state->rel_pos_rem -= floor(state->rel_pos_rem / PI_2) * PI_2;
        }
        ins->bufs[0][i] += val_l;
        ins->bufs[1][i] += val_r;
        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        }
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


