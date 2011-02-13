

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
#include <string.h>

#include <Generator.h>
#include <Generator_common.h>
#include <Device_params.h>
#include <Generator_pulse.h>
#include <Voice_state_pulse.h>
#include <kunquat/limits.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


static void Generator_pulse_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_pulse(uint32_t buffer_size,
                               uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    Generator_pulse* pulse = xalloc(Generator_pulse);
    if (pulse == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&pulse->parent,
                        del_Generator_pulse,
                        Generator_pulse_mix,
                        Generator_pulse_init_state,
                        buffer_size,
                        mix_rate))
    {
        xfree(pulse);
        return NULL;
    }
    pulse->pulse_width = 0.5;
    return &pulse->parent;
}


char* Generator_pulse_property(Generator* gen, const char* property_type)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "pulse"));
    assert(property_type != NULL);
    (void)gen;
    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
        {
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_pulse));
        }
        return size_str;
    }
    return NULL;
}


static void Generator_pulse_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "pulse"));
    assert(state != NULL);
    Voice_state_pulse* pulse_state = (Voice_state_pulse*)state;
    Generator_pulse* pulse = (Generator_pulse*)gen;
    pulse_state->phase = 0;
    pulse_state->pulse_width = pulse->pulse_width;
    return;
}


double pulse(double phase, double pulse_width)
{
    if (phase < pulse_width)
    {
        return 1.0;
    }
    return -1.0;
}


uint32_t Generator_pulse_mix(Generator* gen,
                             Voice_state* state,
                             uint32_t nframes,
                             uint32_t offset,
                             uint32_t freq,
                             double tempo)
{
    assert(gen != NULL);
    assert(string_eq(gen->type, "pulse"));
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen, state, offset, bufs);
    Generator_common_check_active(gen, state, offset);
    Generator_common_check_relative_lengths(gen, state, freq, tempo);
//    double max_amp = 0;
//  fprintf(stderr, "bufs are %p and %p\n", ins->bufs[0], ins->bufs[1]);
    Voice_state_pulse* pulse_state = (Voice_state_pulse*)state;
    if (state->note_on)
    {
        double* pulse_width_arg = Channel_gen_state_get_float(state->cgstate,
                                                        "p_pw.jsonf");
        if (pulse_width_arg != NULL)
        {
            pulse_state->pulse_width = *pulse_width_arg;
        }
        else
        {
            pulse_state->pulse_width = 0.5;
        }
    }
    uint32_t mixed = offset;
    for (; mixed < nframes && state->active; ++mixed)
    {
        Generator_common_handle_pitch(gen, state);

        double vals[KQT_BUFFERS_MAX] = { 0 };
        vals[0] = pulse(pulse_state->phase, pulse_state->pulse_width) / 6;
        Generator_common_handle_force(gen, state, vals, 1, freq);
        Generator_common_handle_filter(gen, state, vals, 1, freq);
        Generator_common_ramp_attack(gen, state, vals, 1, freq);
        pulse_state->phase += state->actual_pitch / freq;
        if (pulse_state->phase >= 1)
        {
            pulse_state->phase -= floor(pulse_state->phase);
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


void del_Generator_pulse(Generator* gen)
{
    if (gen == NULL)
    {
        return;
    }
    assert(string_eq(gen->type, "pulse"));
    Generator_pulse* pulse = (Generator_pulse*)gen;
    xfree(pulse);
    return;
}


