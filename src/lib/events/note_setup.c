

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Instrument.h>
#include <kunquat/limits.h>
#include <note_setup.h>
#include <player/Channel.h>
#include <Slider.h>
#include <xassert.h>


void reserve_voice(Channel* ch, Instrument* ins, int gen_num)
{
    assert(ch != NULL);
    assert(ch->freq != NULL);
    assert(*ch->freq > 0);
    assert(ch->tempo != NULL);
    assert(*ch->tempo > 0);
    assert(ins != NULL);
    assert(gen_num >= 0);
    assert(gen_num < KQT_GENERATORS_MAX);

    ++ch->fg_count;
    ch->fg[gen_num] = Voice_pool_get_voice(ch->pool, NULL, 0);
    assert(ch->fg[gen_num] != NULL);
//    fprintf(stderr, "allocated Voice %p\n", (void*)ch->fg[gen_num]);
    ch->fg_id[gen_num] = Voice_id(ch->fg[gen_num]);

    Voice_init(ch->fg[gen_num],
               Instrument_get_gen(ins, gen_num),
               &ch->vp,
               ch->cgstate,
               Random_get_uint64(ch->rand),
               *ch->freq,
               *ch->tempo);

    return;
}


void set_instrument_properties(
        Voice* voice,
        Voice_state* vs,
        Channel* ch,
        double* force_var)
{
    assert(force_var != NULL);

    vs->sustain = &voice->gen->ins_params->sustain;
    vs->force = exp2(voice->gen->ins_params->force / 6);

    if (voice->gen->ins_params->force_variation != 0)
    {
        if (isnan(*force_var))
        {
            double var_dB = Random_get_float_scale(ch->rand) *
                            voice->gen->ins_params->force_variation;
            var_dB -= voice->gen->ins_params->force_variation / 2;
            *force_var = exp2(var_dB / 6);
        }
        vs->force *= *force_var;
        vs->actual_force = vs->force * voice->gen->ins_params->global_force;
    }

    Slider_set_length(&vs->force_slider, &ch->force_slide_length);
//    LFO_copy(&vs->tremolo, &ch->tremolo);
    Slider_set_length(&vs->pitch_slider, &ch->pitch_slide_length);
//    LFO_copy(&vs->vibrato, &ch->vibrato);
    vs->panning = ch->panning;
    Slider_copy(&vs->panning_slider, &ch->panning_slider);
    Slider_set_length(&vs->lowpass_slider, &ch->filter_slide_length);
//    LFO_copy(&vs->autowah, &ch->autowah);

    return;
}


