

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <Generator.h>
#include <Generator_common.h>
#include <Generator_sine.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <memory.h>
#include <string_common.h>
#include <Voice_state_sine.h>
#include <xassert.h>


void Generator_sine_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_sine(uint32_t buffer_size,
                              uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    Generator_sine* sine = memory_alloc_item(Generator_sine);
    if (sine == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&sine->parent,
                        del_Generator_sine,
                        Generator_sine_mix,
                        Generator_sine_init_state,
                        buffer_size,
                        mix_rate))
    {
        memory_free(sine);
        return NULL;
    }
    return &sine->parent;
}


void Generator_sine_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "sine"));
    assert(state != NULL);
    (void)gen;
    Voice_state_sine* sine_state = (Voice_state_sine*)state;
    sine_state->phase = 0;
    return;
}


char* Generator_sine_property(Generator* gen, const char* property_type)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "sine"));
    assert(property_type != NULL);
    (void)gen;
    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
        {
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_sine));
        }
        return size_str;
    }
    return NULL;
}


uint32_t Generator_sine_mix(Generator* gen,
                            Voice_state* state,
                            uint32_t nframes,
                            uint32_t offset,
                            uint32_t freq,
                            double tempo)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "sine"));
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen, state, offset, bufs);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_sine* sine_state = (Voice_state_sine*)state;
    uint32_t mixed = offset;
    for (; mixed < nframes && state->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, state);

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = sin(sine_state->phase * PI * 2) / 6;
        Generator_common_handle_force(gen, state, vals, 1, freq);
        Generator_common_handle_filter(gen, state, vals, 1, freq);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
        sine_state->phase += state->actual_pitch / freq;
        if (sine_state->phase >= 1)
        {
            sine_state->phase -= floor(sine_state->phase);
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


void del_Generator_sine(Generator* gen)
{
    if (gen == NULL)
    {
        return;
    }
    assert(string_eq(gen->type, "sine"));
    Generator_sine* sine = (Generator_sine*)gen;
    memory_free(sine);
    return;
}


