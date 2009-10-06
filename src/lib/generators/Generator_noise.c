

/*
 * Copyright 2009 Tomi Jylhä-Ollila, Ossi Saresoja
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
#include <Generator_noise.h>
#include <kunquat/limits.h>
#include <math_common.h>

#include <xmemory.h>

#define RAND_U ((double)((rand() << 1) - RAND_MAX)/RAND_MAX)


static bool Generator_noise_read(Generator* gen, File_tree* tree, Read_state* state);

void Generator_noise_init_state(Generator* gen, Voice_state* state);


Generator_noise* new_Generator_noise(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_noise* noise = xalloc(Generator_noise);
    if (noise == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&noise->parent))
    {
        xfree(noise);
        return NULL;
    }
    noise->parent.read = Generator_noise_read;
    noise->parent.destroy = del_Generator_noise;
    noise->parent.type = GEN_TYPE_NOISE;
    noise->parent.init_state = Generator_noise_init_state;
    noise->parent.mix = Generator_noise_mix;
    noise->parent.ins_params = ins_params;
    return noise;
}


static bool Generator_noise_read(Generator* gen, File_tree* tree, Read_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
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


void Generator_noise_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
    assert(state != NULL);
    (void)gen;
    (void)state;
//    Voice_state_noise* noise_state = (Voice_state_noise*)state;
//    noise_state->phase = 0;
    return;
}


uint32_t Generator_noise_mix(Generator* gen,
                            Voice_state* state,
                            uint32_t nframes,
                            uint32_t offset,
                            uint32_t freq,
                            double tempo,
                            int buf_count,
                            kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
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
//    Voice_state_noise* noise_state = (Voice_state_noise*)state;
    uint32_t mixed = offset;
    for (; mixed < nframes; ++mixed)
    {
        Generator_common_handle_filter(gen, state);
        Generator_common_handle_pitch(gen, state);

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = RAND_U;
	vals[1] = RAND_U;
        Generator_common_handle_force(gen, state, vals, 2);
        Generator_common_ramp_attack(gen, state, vals, 2, freq);
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


void del_Generator_noise(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_NOISE);
    Generator_noise* noise = (Generator_noise*)gen;
    xfree(noise);
    return;
}
