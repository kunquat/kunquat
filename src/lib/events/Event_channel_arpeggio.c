

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
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_channel_arpeggio.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <float.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc arpeggio_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = DBL_MIN,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -DBL_MAX,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -DBL_MAX,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -DBL_MAX,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_channel_arpeggio_set(Event* event, int index, void* data);

static void* Event_channel_arpeggio_get(Event* event, int index);


Event_create_constructor(Event_channel_arpeggio,
                         EVENT_CHANNEL_ARPEGGIO,
                         arpeggio_desc,
                         for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i) { event->notes[i] = 0; }
                         event->speed = 24);


static bool Event_channel_arpeggio_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_ARPEGGIO);
    assert(data != NULL);
    Event_channel_arpeggio* arpeggio = (Event_channel_arpeggio*)event;
    if (index == 0)
    {
        double speed = *(double*)data;
        Event_check_double_range(speed, event->field_types[0]);
        arpeggio->speed = speed;
        return true;
    }
    else if (index >= 1 && index < KQT_ARPEGGIO_NOTES_MAX + 1)
    {
        double note = *(double*)data;
        Event_check_double_range(note, event->field_types[index]);
        arpeggio->notes[index - 1] = note;
        return true;
    }
    return false;
}


static void* Event_channel_arpeggio_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_ARPEGGIO);
    Event_channel_arpeggio* arpeggio = (Event_channel_arpeggio*)event;
    if (index == 0)
    {
        return &arpeggio->speed;
    }
    else if (index >= 1 && index < KQT_ARPEGGIO_NOTES_MAX + 1)
    {
        return &arpeggio->notes[index - 1];
    }
    return NULL;
}


bool Event_channel_arpeggio_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[4];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, arpeggio_desc, data, state);
    if (state->error)
    {
        return false;
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;
        pitch_t orig_pitch = -1;
        if (voice->gen->ins_params->pitch_lock_enabled)
        {
            return true;
        }
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
        double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                           *ch_state->tempo,
                                           *ch_state->freq);
        vs->arpeggio_length = unit_len / data[0].field.double_type;
        vs->arpeggio_frames = 0;
        vs->arpeggio_note = 0;
        vs->arpeggio = true;
    }
    return true;
}


