

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <debug/assert.h>
#include <devices/Audio_unit.h>
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <player/events/note_setup.h>
#include <player/Slider.h>


void reserve_voice(
        Channel* ch,
        Audio_unit* au,
        uint64_t group_id,
        const Proc_state* proc_state,
        int proc_num,
        uint64_t rand_seed)
{
    assert(ch != NULL);
    assert(ch->freq != NULL);
    assert(*ch->freq > 0);
    assert(ch->tempo != NULL);
    assert(*ch->tempo > 0);
    assert(au != NULL);
    assert(proc_state != NULL);
    assert(proc_num >= 0);
    assert(proc_num < KQT_PROCESSORS_MAX);

    ++ch->fg_count;
    ch->fg[proc_num] = Voice_pool_get_voice(ch->pool, NULL, 0);
    assert(ch->fg[proc_num] != NULL);
//    fprintf(stderr, "allocated Voice %p\n", (void*)ch->fg[proc_num]);
    ch->fg_id[proc_num] = Voice_id(ch->fg[proc_num]);

    Voice_init(ch->fg[proc_num],
               Audio_unit_get_proc(au, proc_num),
               group_id,
               proc_state,
               ch->cpstate,
               rand_seed,
               *ch->freq,
               *ch->tempo);

    return;
}


void set_au_properties(Voice* voice, Voice_state* vs, Channel* ch, double* force_var)
{
    assert(force_var != NULL);

    vs->force = exp2(voice->proc->au_params->force / 6);

    if (voice->proc->au_params->force_variation != 0)
    {
        if (isnan(*force_var))
        {
            double var_dB = Random_get_float_scale(ch->rand) *
                            voice->proc->au_params->force_variation;
            var_dB -= voice->proc->au_params->force_variation / 2;
            *force_var = exp2(var_dB / 6);
        }
        vs->force *= *force_var;
        vs->actual_force = vs->force * voice->proc->au_params->global_force;
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


