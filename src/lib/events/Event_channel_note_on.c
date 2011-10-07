

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
#include <note_setup.h>
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
        reserve_voice(ch_state, ins, i);

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
        vs->orig_cents = data[0].field.double_type;

        set_instrument_properties(voice, vs, ch_state, &force_var);
    }
    return true;
}


