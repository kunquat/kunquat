

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
#include <string.h>
#include <math.h>
#include <float.h>

#include <Event_common.h>
#include <Event_channel_arpeggio_on.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <Scale.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc arpeggio_on_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_ARPEGGIO_ON,
                         arpeggio_on);


bool Event_channel_arpeggio_on_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;
        //pitch_t orig_pitch = -1;
        if (vs->arpeggio || voice->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
#if 0
        if (voice->gen->ins_params->scale != NULL &&
                *voice->gen->ins_params->scale != NULL &&
                **voice->gen->ins_params->scale != NULL)
        {
            orig_pitch = Scale_get_pitch_from_cents(
                         **voice->gen->ins_params->scale, vs->orig_cents);
        }
        else
        {
            orig_pitch = exp2(vs->orig_cents / 1200) * 440;
        }
        if (orig_pitch <= 0)
        {
            vs->arpeggio = false;
            continue;
        }
#endif
        if (isnan(ch_state->arpeggio_ref))
        {
            ch_state->arpeggio_ref = vs->orig_cents;
        }
        if (isnan(ch_state->arpeggio_tones[0]))
        {
            ch_state->arpeggio_tones[0] = ch_state->arpeggio_ref;
        }
        vs->arpeggio_ref = ch_state->arpeggio_ref;
        memcpy(vs->arpeggio_tones, ch_state->arpeggio_tones,
                KQT_ARPEGGIO_NOTES_MAX * sizeof(double));
#if 0
        int last_nonzero = -1;
        for (int k = 0; k < KQT_ARPEGGIO_NOTES_MAX; ++k)
        {
            if (data[k + 1].field.double_type != 0)
            {
                last_nonzero = k;
            }
            pitch_t new_pitch = -1;
            if (voice->gen->ins_params->scale != NULL &&
                    *voice->gen->ins_params->scale != NULL &&
                    **voice->gen->ins_params->scale != NULL)
            {
                Scale* scale = **voice->gen->ins_params->scale;
                new_pitch = Scale_get_pitch_from_cents(scale,
                            vs->orig_cents + data[k + 1].field.double_type);
            }
            else
            {
                new_pitch = vs->orig_cents + data[k + 1].field.double_type;
            }
            if (new_pitch <= 0)
            {
                last_nonzero = -1;
                break;
            }
            else
            {
                vs->arpeggio_factors[k] = new_pitch / orig_pitch;
            }
        }
        if (last_nonzero == -1)
        {
            vs->arpeggio = false;
            continue;
        }
        else if (last_nonzero < KQT_ARPEGGIO_NOTES_MAX - 1)
        {
            vs->arpeggio_factors[last_nonzero + 1] = -1;
        }
#endif
        double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                           *ch_state->tempo,
                                           *ch_state->freq);
        vs->arpeggio_length = unit_len / ch_state->arpeggio_speed;
        vs->arpeggio_frames = 0;
        vs->arpeggio_note = 0;
        vs->arpeggio = true;
    }
    return true;
}


