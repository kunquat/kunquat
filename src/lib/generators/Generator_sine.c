

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
#include <Generator_sine.h>
#include <Voice_state_sine.h>
#include <kunquat/limits.h>
#include <math_common.h>

#include <xmemory.h>


void Generator_sine_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_sine(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_sine* sine = xalloc(Generator_sine);
    if (sine == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&sine->parent))
    {
        xfree(sine);
        return NULL;
    }
    sine->parent.destroy = del_Generator_sine;
    sine->parent.type = GEN_TYPE_SINE;
    sine->parent.init_state = Generator_sine_init_state;
    sine->parent.mix = Generator_sine_mix;
    sine->parent.ins_params = ins_params;
    return &sine->parent;
}


void Generator_sine_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
    assert(state != NULL);
    (void)gen;
    Voice_state_sine* sine_state = (Voice_state_sine*)state;
    sine_state->phase = 0;
    return;
}


uint32_t Generator_sine_mix(Generator* gen,
                            Voice_state* state,
                            uint32_t nframes,
                            uint32_t offset,
                            uint32_t freq,
                            double tempo,
                            int buf_count,
                            kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
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
    Voice_state_sine* sine_state = (Voice_state_sine*)state;
    uint32_t mixed = offset;
    for (; mixed < nframes; ++mixed)
    {
        Generator_common_handle_filter(gen, state);
        Generator_common_handle_pitch(gen, state);

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = sin(sine_state->phase * PI * 2) / 6;
        sine_state->phase += state->actual_pitch / freq;
        if (sine_state->phase >= 1)
        {
            sine_state->phase -= floor(sine_state->phase);
        }
        state->pos = 1; // XXX: hackish
        Generator_common_handle_force(gen, state, vals, 1);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
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


void del_Generator_sine(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_SINE);
    Generator_sine* sine = (Generator_sine*)gen;
    xfree(sine);
    return;
}


