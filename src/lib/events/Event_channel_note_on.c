

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
#include <stdbool.h>
#include <float.h>

#include <Event_common.h>
#include <Event_channel_note_on.h>
#include <Event_channel_note_off.h>
#include <Random.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>

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


Event_create_set_primitive_and_get(Event_channel_note_on,
                                   EVENT_CHANNEL_NOTE_ON,
                                   double, cents);


Event_create_constructor(Event_channel_note_on,
                         EVENT_CHANNEL_NOTE_ON,
                         note_on_desc,
                         event->cents = 0);


bool Event_channel_note_on_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
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
    ch_state->panning_slide = 0;
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
                   *ch_state->freq,
                   *ch_state->tempo);
        Voice_pool_fix_priority(ch_state->pool, ch_state->fg[i]);

        Voice* voice = ch_state->fg[i];
        Voice_state* vs = &voice->state.generic;
        Generator_process_note(voice->gen,
                               vs,
                               data[0].field.double_type);
        vs->orig_cents = data[0].field.double_type;

        vs->pedal = &voice->gen->ins_params->pedal;
        if (voice->gen->ins_params->force_variation != 0)
        {
            if (isnan(force_var))
            {
                double var_dB = ((double)Random_get(voice->gen->random) /
                                 KQT_RANDOM_MAX) *
                                voice->gen->ins_params->force_variation;
                var_dB -= voice->gen->ins_params->force_variation / 2;
                force_var = exp2(var_dB / 6);
            }
            vs->force *= force_var;
        }

        Reltime_copy(&vs->force_slide_length, &ch_state->force_slide_length);
        vs->tremolo_length = ch_state->tremolo_length;
        vs->tremolo_update = ch_state->tremolo_update;
        vs->tremolo_depth_target = ch_state->tremolo_depth;
        vs->tremolo_delay_update = ch_state->tremolo_delay_update;

        Reltime_copy(&vs->pitch_slide_length, &ch_state->pitch_slide_length);
        vs->vibrato_length = ch_state->vibrato_length;
        vs->vibrato_update = ch_state->vibrato_update;
        vs->vibrato_depth_target = ch_state->vibrato_depth;
        vs->vibrato_delay_update = ch_state->vibrato_delay_update;

        vs->panning = ch_state->panning;
        vs->panning_slide = ch_state->panning_slide;
        Reltime_copy(&vs->panning_slide_length, &ch_state->panning_slide_length);
        vs->panning_slide_target = ch_state->panning_slide_target;
        vs->panning_slide_frames = ch_state->panning_slide_frames;
        vs->panning_slide_update = ch_state->panning_slide_update;

        Reltime_copy(&vs->filter_slide_length, &ch_state->filter_slide_length);
    }
    return true;
}


