

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
#include <stdint.h>
#include <math.h>

#include "Generator_debug.h"

#include <xmemory.h>


Generator_debug* new_Generator_debug(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_debug* debug = xalloc(Generator_debug);
    if (debug == NULL)
    {
        return NULL;
    }
    debug->parent.destroy = del_Generator_debug;
    debug->parent.type = GEN_TYPE_DEBUG;
    debug->parent.init_state = NULL;
    debug->parent.mix = Generator_debug_mix;
    debug->parent.ins_params = ins_params;
    return debug;
}


void Generator_debug_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_DEBUG);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); // XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(gen->ins_params->bufs[0] != NULL);
    assert(gen->ins_params->bufs[1] != NULL);
    if (!state->active)
    {
        return;
    }
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double val_l = 0;
        double val_r = 0;
        if (state->rel_pos == 0)
        {
            val_l = 1.0;
            val_r = 1.0;
            state->rel_pos = 1;
        }
        else
        {
            val_l = 0.5;
            val_r = 0.5;
        }
        if (!state->note_on)
        {
            val_l = -val_l;
            val_r = -val_r;
        }
        gen->ins_params->bufs[0][i] += val_l;
        gen->ins_params->bufs[1][i] += val_r;
        state->rel_pos_rem += state->freq / freq;
        if (!state->note_on)
        {
            state->noff_pos_rem += state->freq / freq;
            if (state->noff_pos_rem >= 2)
            {
                state->active = false;
                return;
            }
        }
        if (state->rel_pos_rem >= 1)
        {
            ++state->pos;
            if (state->pos >= 10)
            {
                state->active = false;
                return;
            }
            state->rel_pos = 0;
            state->rel_pos_rem -= floor(state->rel_pos_rem);
        }
    }
    return;
}


void del_Generator_debug(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_DEBUG);
    Generator_debug* debug = (Generator_debug*)gen;
    xfree(debug);
    return;
}


