

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
#include <stdbool.h>
#include <stdio.h>
#include <float.h>

#include <Event_common.h>
#include <Event_channel_note_on.h>
#include <Event_channel_note_off.h>
#include <Random.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc note_on_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -DBL_MAX,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_NOTE_ON,
                         note_on);


bool Event_channel_note_on_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    assert(ch_state->freq != NULL);
    assert(ch_state->tempo != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, note_on_desc, data, state);
    if (state->error)
    {
        return false;
    }
    // move the old Voices to the background
    Event_channel_note_off_process(ch_state, NULL);
    ch_state->fg_count = 0;
    assert(ch_state->instrument >= 0);
    assert(ch_state->instrument < KQT_INSTRUMENTS_MAX);
    Instrument* ins = Ins_table_get(ch_state->insts,
                                    ch_state->instrument);
    if (ins == NULL)
    {
        return true;
    }
    // allocate new Voices
//    ch_state->panning_slide = 0;
    double force_var = NAN;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (Instrument_get_gen(ins, i) == NULL)
        {
            continue;
        }
        ++ch_state->fg_count;
        ch_state->fg[i] = Voice_pool_get_voice(ch_state->pool, NULL, 0);
        assert(ch_state->fg[i] != NULL);
//        fprintf(stderr, "allocated Voice %p\n", (void*)ch_state->fg[i]);
        ch_state->fg_id[i] = Voice_id(ch_state->fg[i]);
        Voice_init(ch_state->fg[i],
                   Instrument_get_gen(ins, i),
                   &ch_state->vp,
                   ch_state->cgstate,
                   Random_get_uint64(ch_state->rand),
                   *ch_state->freq,
                   *ch_state->tempo);
        Voice_pool_fix_priority(ch_state->pool, ch_state->fg[i]);

        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;

        if (voice->gen->ins_params->pitch_locks[i].enabled)
        {
            vs->pitch = voice->gen->ins_params->pitch_locks[i].freq;
        }
        else if (voice->gen->ins_params->scale == NULL ||
                 *voice->gen->ins_params->scale == NULL ||
                 **voice->gen->ins_params->scale == NULL)
        {
            vs->pitch = exp2(data[0].field.double_type / 1200) * 440;
        }
        else
        {
            pitch_t pitch = Scale_get_pitch_from_cents(
                                    **voice->gen->ins_params->scale,
                                    data[0].field.double_type);
            if (pitch > 0)
            {
                vs->pitch = pitch;
            }
            else
            {
                vs->pitch = exp2(data[0].field.double_type / 1200) * 440;
            }
        }
        //fprintf(stderr, "Event set pitch @ %p: %f\n", (void*)&vs->pitch, vs->pitch);
#if 0
        Generator_process_note(voice->gen,
                               vs,
                               data[0].field.double_type);
#endif
        vs->orig_cents = data[0].field.double_type;

        vs->pedal = &voice->gen->ins_params->pedal;
        vs->force = exp2(voice->gen->ins_params->force / 6);
        if (voice->gen->ins_params->force_variation != 0)
        {
            if (isnan(force_var))
            {
                double var_dB = Random_get_float_scale(ch_state->rand) *
                                voice->gen->ins_params->force_variation;
                var_dB -= voice->gen->ins_params->force_variation / 2;
                force_var = exp2(var_dB / 6);
            }
            vs->force *= force_var;
        }

        Slider_set_length(&vs->force_slider, &ch_state->force_slide_length);
//        LFO_copy(&vs->tremolo, &ch_state->tremolo);

        Slider_set_length(&vs->pitch_slider, &ch_state->pitch_slide_length);
//        LFO_copy(&vs->vibrato, &ch_state->vibrato);

        vs->panning = ch_state->panning;
        Slider_copy(&vs->panning_slider, &ch_state->panning_slider);

        Slider_set_length(&vs->lowpass_slider, &ch_state->filter_slide_length);
//        LFO_copy(&vs->autowah, &ch_state->autowah);
    }
    return true;
}


