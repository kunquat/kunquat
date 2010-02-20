

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
#include <math.h>

#include <Event_common.h>
#include <Event_channel_slide_pitch.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_pitch_desc[] =
{
    {
        .type = EVENT_FIELD_NOTE,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_SCALE_NOTES - 1
    },
    {
        .type = EVENT_FIELD_NOTE_MOD,
        .min.field.integral_type = -1,
        .max.field.integral_type = KQT_SCALE_NOTE_MODS - 1
    },
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = KQT_SCALE_OCTAVE_FIRST,
        .max.field.integral_type = KQT_SCALE_OCTAVE_LAST
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_channel_slide_pitch_set(Event* event, int index, void* data);

static void* Event_channel_slide_pitch_get(Event* event, int index);


Event_create_constructor(Event_channel_slide_pitch,
                         EVENT_CHANNEL_SLIDE_PITCH,
                         slide_pitch_desc,
                         event->note = 0,
                         event->mod = -1,
                         event->octave = KQT_SCALE_MIDDLE_OCTAVE);


static bool Event_channel_slide_pitch_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_SLIDE_PITCH);
    assert(data != NULL);
    Event_channel_slide_pitch* slide_pitch = (Event_channel_slide_pitch*)event;
    switch (index)
    {
        case 0:
        {
            int64_t num = *(int64_t*)data;
            Event_check_integral_range(num, event->field_types[0]);
            slide_pitch->note = num;
            return true;
        }
        break;
        case 1:
        {
            int64_t num = *(int64_t*)data;
            Event_check_integral_range(num, event->field_types[1]);
            slide_pitch->mod = num;
            return true;
        }
        break;
        case 2:
        {
            int64_t num = *(int64_t*)data;
            Event_check_integral_range(num, event->field_types[2]);
            slide_pitch->octave = num;
            return true;
        }
        break;
        default:
        break;
    }
    return false;
}


static void* Event_channel_slide_pitch_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_SLIDE_PITCH);
    Event_channel_slide_pitch* slide_pitch = (Event_channel_slide_pitch*)event;
    switch (index)
    {
        case 0:
        {
            return &slide_pitch->note;
        }
        case 1:
        {
            return &slide_pitch->mod;
        }
        case 2:
        {
            return &slide_pitch->octave;
        }
        default:
        break;
    }
    return NULL;
}


bool Event_channel_slide_pitch_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[3];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_pitch_desc, data, state);
    if (state->error)
    {
        return false;
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice* voice = ch_state->fg[i];
        Voice_state* vs = &voice->state.generic;
        if (voice->gen->ins_params->scale == NULL ||
                *voice->gen->ins_params->scale == NULL ||
                **voice->gen->ins_params->scale == NULL)
        {
            continue;
        }
        pitch_t pitch = Scale_get_pitch(**voice->gen->ins_params->scale,
                                        data[0].field.integral_type,
                                        data[1].field.integral_type,
                                        data[2].field.integral_type);
        if (pitch <= 0)
        {
            continue;
        }
        vs->pitch_slide_frames = Reltime_toframes(&vs->pitch_slide_length,
                                                  *ch_state->tempo,
                                                  *ch_state->freq);
        vs->pitch_slide_target = pitch;
        double diff_log = log2(pitch) - log2(vs->pitch);
        double slide_step = diff_log / vs->pitch_slide_frames;
        vs->pitch_slide_update = exp2(slide_step);
        if (slide_step > 0)
        {
            vs->pitch_slide = 1;
        }
        else if (slide_step < 0)
        {
            vs->pitch_slide = -1;
        }
        else
        {
            vs->pitch = vs->pitch_slide_target;
            vs->pitch_slide = 0;
        }
    }
    return true;
}


