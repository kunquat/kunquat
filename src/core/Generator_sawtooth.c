

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
#include <Generator_sawtooth.h>
#include <Voice_state_sawtooth.h>

#include <xmemory.h>


#define PI_2 (3.14159265358979323846 * 2)


Generator_sawtooth* new_Generator_sawtooth(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_sawtooth* sawtooth = xalloc(Generator_sawtooth);
    if (sawtooth == NULL)
    {
        return NULL;
    }
    sawtooth->parent.destroy = del_Generator_sawtooth;
    sawtooth->parent.type = GEN_TYPE_SAWTOOTH;
    sawtooth->parent.init_state = Voice_state_sawtooth_init;
    sawtooth->parent.mix = Generator_sawtooth_mix;
    sawtooth->parent.ins_params = ins_params;
    return sawtooth;
}

double sawtooth(double x)
{
    return atan(tan(x/2));
} 

void Generator_sawtooth_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SAWTOOTH);
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
    Voice_state_sawtooth* sawtooth_state = (Voice_state_sawtooth*)state;
    for (uint32_t i = offset; i < nframes; ++i)
    {
        double val_l = 0;
        double val_r = 0;
        val_l = val_r = sawtooth(sawtooth_state->phase) / 6;
        if (!state->note_on && (state->pos_rem == 0)
                && !gen->ins_params->volume_off_env_enabled)
        {
            return;
        }
        if (state->pos_rem < 0.002)
        {
            val_l = val_r = val_l * (state->pos_rem * 500);
            state->pos_rem += 1.0 / freq;
        }
        sawtooth_state->phase += state->freq * PI_2 / freq;
        if (sawtooth_state->phase >= PI_2)
        {
            sawtooth_state->phase -= floor(sawtooth_state->phase / PI_2) * PI_2;
            ++state->pos;
        }
        if (!state->note_on)
        {
            if (gen->ins_params->volume_off_env_enabled)
            {
                double scale = Envelope_get_value(gen->ins_params->volume_off_env,
                        state->off_ve_pos);
                if (!isfinite(scale))
                {
                    state->active = false;
                    return;
                }
                if (state->pedal < 0.5)
                {
                    state->off_ve_pos += 1.0 / freq;
                }
                val_l *= scale;
                val_r *= scale;
            }
            else
            {
                if (state->noff_pos_rem < 1)
                {
                    val_l = val_r = val_l * (1 - state->noff_pos_rem);
                }
                else
                {
                    state->active = false;
                    return;
                }
                state->noff_pos_rem += 2.0 / freq;
            }
        }
        gen->ins_params->bufs[0][i] += val_l;
        gen->ins_params->bufs[1][i] += val_r;
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
    return;
}


void del_Generator_sawtooth(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SAWTOOTH);
    Generator_sawtooth* sawtooth = (Generator_sawtooth*)gen;
    xfree(sawtooth);
    return;
}


