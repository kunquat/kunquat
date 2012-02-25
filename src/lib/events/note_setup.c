

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Channel_state.h>
#include <Instrument.h>
#include <kunquat/limits.h>
#include <note_setup.h>
#include <Slider.h>
#include <xassert.h>


void reserve_voice(Channel_state* ch_state,
                   Instrument* ins,
                   int gen_num)
{
    assert(ch_state != NULL);
    assert(ins != NULL);
    assert(gen_num >= 0);
    assert(gen_num < KQT_GENERATORS_MAX);
    ++ch_state->fg_count;
    ch_state->fg[gen_num] = Voice_pool_get_voice(ch_state->pool, NULL, 0);
    assert(ch_state->fg[gen_num] != NULL);
//    fprintf(stderr, "allocated Voice %p\n", (void*)ch_state->fg[gen_num]);
    ch_state->fg_id[gen_num] = Voice_id(ch_state->fg[gen_num]);
    Voice_init(ch_state->fg[gen_num],
               Instrument_get_gen(ins, gen_num),
               &ch_state->vp,
               ch_state->cgstate,
               Random_get_uint64(ch_state->rand),
               *ch_state->freq,
               *ch_state->tempo);
    Voice_pool_fix_priority(ch_state->pool, ch_state->fg[gen_num]);
    return;
}


void set_instrument_properties(Voice* voice,
                               Voice_state* vs,
                               Channel_state* ch_state,
                               double* force_var)
{
    assert(force_var != NULL);
    vs->sustain = &voice->gen->ins_params->sustain;
    vs->force = exp2(voice->gen->ins_params->force / 6);
    if (voice->gen->ins_params->force_variation != 0)
    {
        if (isnan(*force_var))
        {
            double var_dB = Random_get_float_scale(ch_state->rand) *
                            voice->gen->ins_params->force_variation;
            var_dB -= voice->gen->ins_params->force_variation / 2;
            *force_var = exp2(var_dB / 6);
        }
        vs->force *= *force_var;
        vs->actual_force = vs->force * voice->gen->ins_params->global_force;
    }
    Slider_set_length(&vs->force_slider, &ch_state->force_slide_length);
//    LFO_copy(&vs->tremolo, &ch_state->tremolo);
    Slider_set_length(&vs->pitch_slider, &ch_state->pitch_slide_length);
//    LFO_copy(&vs->vibrato, &ch_state->vibrato);
    vs->panning = ch_state->panning;
    Slider_copy(&vs->panning_slider, &ch_state->panning_slider);
    Slider_set_length(&vs->lowpass_slider, &ch_state->filter_slide_length);
//    LFO_copy(&vs->autowah, &ch_state->autowah);
    return;
}


