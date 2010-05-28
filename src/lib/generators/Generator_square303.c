

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
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
#include <kunquat/limits.h>

#include <xmemory.h>


void Generator_square303_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_square303(Instrument_params* ins_params,
                                   Generator_params* gen_params)
{
    assert(ins_params != NULL);
    assert(gen_params != NULL);
    Generator_square303* square303 = xalloc(Generator_square303);
    if (square303 == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&square303->parent))
    {
        xfree(square303);
        return NULL;
    }
    square303->parent.destroy = del_Generator_square303;
    square303->parent.type = GEN_TYPE_SQUARE303;
    square303->parent.init_state = Generator_square303_init_state;
    square303->parent.mix = Generator_square303_mix;
    square303->parent.ins_params = ins_params;
    square303->parent.type_params = gen_params;
    return &square303->parent;
}


void Generator_square303_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE303);
    (void)gen;
    assert(state != NULL);
    Voice_state_square303* square303_state = (Voice_state_square303*)state;
    square303_state->phase = 0.5;
    return;
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


uint32_t Generator_square303_mix(Generator* gen,
                                 Voice_state* state,
                                 uint32_t nframes,
                                 uint32_t offset,
                                 uint32_t freq,
                                 double tempo,
                                 int buf_count,
                                 kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE303);
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
    Voice_state_square303* square303_state = (Voice_state_square303*)state;
    uint32_t mixed = offset;
    for (; mixed < nframes && state->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, state);
        
        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = square303(square303_state->phase) / 6;
        Generator_common_handle_force(gen, state, vals, 1, freq);
        Generator_common_handle_filter(gen, state, vals, 1, freq);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
        square303_state->phase += state->actual_pitch / freq;
        if (square303_state->phase >= 1)
        {
            square303_state->phase -= floor(square303_state->phase);
        }
        state->pos = 1; // XXX: hackish
//        Generator_common_handle_note_off(gen, state, vals, 1, freq);
        vals[1] = vals[0];
        Generator_common_handle_panning(gen, state, vals, 2);
        bufs[0][mixed] += vals[0];
        bufs[1][mixed] += vals[1];
/*        if (fabs(val_l) > max_amp)
        {
            max_amp = fabs(val_l);
        } */
    }
//  fprintf(stderr, "max_amp is %lf\n", max_amp);
//    Generator_common_persist(gen, state, mixed);
    return mixed;
}


void del_Generator_square303(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SQUARE303);
    Generator_square303* square303 = (Generator_square303*)gen;
    xfree(square303);
    return;
}


