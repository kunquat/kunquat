

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_slide_pitch.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_pitch_desc[] =
{
    {
        .type = EVENT_FIELD_NOTE,
        .range.integral_type = { 0, KQT_SCALE_NOTES - 1 }
    },
    {
        .type = EVENT_FIELD_NOTE_MOD,
        .range.integral_type = { -1, KQT_SCALE_NOTE_MODS - 1 }
    },
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { KQT_SCALE_OCTAVE_FIRST, KQT_SCALE_OCTAVE_LAST }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_slide_pitch_set(Event* event, int index, void* data);

static void* Event_voice_slide_pitch_get(Event* event, int index);

static void Event_voice_slide_pitch_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_pitch,
                         EVENT_VOICE_SLIDE_PITCH,
                         slide_pitch_desc,
                         event->note = 0,
                         event->mod = -1,
                         event->octave = KQT_SCALE_MIDDLE_OCTAVE)


static bool Event_voice_slide_pitch_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SLIDE_PITCH);
    assert(data != NULL);
    Event_voice_slide_pitch* slide_pitch = (Event_voice_slide_pitch*)event;
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


static void* Event_voice_slide_pitch_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SLIDE_PITCH);
    Event_voice_slide_pitch* slide_pitch = (Event_voice_slide_pitch*)event;
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


static void Event_voice_slide_pitch_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_PITCH);
    assert(voice != NULL);
    Event_voice_slide_pitch* slide_pitch = (Event_voice_slide_pitch*)event;
    if (voice->gen->ins_params->scale == NULL ||
            *voice->gen->ins_params->scale == NULL ||
            **voice->gen->ins_params->scale == NULL)
    {
        return;
    }
    pitch_t pitch = Scale_get_pitch(**voice->gen->ins_params->scale,
                                    slide_pitch->note,
                                    slide_pitch->mod,
                                    slide_pitch->octave);
    if (pitch <= 0)
    {
        return;
    }
    voice->state.generic.pitch_slide_frames =
            Reltime_toframes(&voice->state.generic.pitch_slide_length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    voice->state.generic.pitch_slide_target = pitch;
    double diff_log = log2(pitch) - log2(voice->state.generic.pitch);
    double slide_step = diff_log / voice->state.generic.pitch_slide_frames;
    voice->state.generic.pitch_slide_update = exp2(slide_step);
    if (slide_step > 0)
    {
        voice->state.generic.pitch_slide = 1;
    }
    else if (slide_step < 0)
    {
        voice->state.generic.pitch_slide = -1;
    }
    else
    {
        voice->state.generic.pitch = voice->state.generic.pitch_slide_target;
    }
    return;
}


