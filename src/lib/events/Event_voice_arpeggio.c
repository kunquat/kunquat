

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_arpeggio.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <float.h>

#include <xmemory.h>


static Event_field_desc arpeggio_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { DBL_MIN, DBL_MAX }
    },
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { -255, 255 }
    },
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { -255, 255 }
    },
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { -255, 255 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_arpeggio_set(Event* event, int index, void* data);

static void* Event_voice_arpeggio_get(Event* event, int index);

static void Event_voice_arpeggio_process(Event_voice* event, Voice* voice);


create_constructor(Event_voice_arpeggio,
                   EVENT_VOICE_ARPEGGIO,
                   arpeggio_desc,
                   for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i) { event->notes[i] = 0; }
                   event->speed = 24)


static bool Event_voice_arpeggio_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_ARPEGGIO);
    assert(data != NULL);
    Event_voice_arpeggio* arpeggio = (Event_voice_arpeggio*)event;
    if (index == 0)
    {
        double speed = *(double*)data;
        Event_check_double_range(speed, event->field_types[0]);
        arpeggio->speed = speed;
        return true;
    }
    else if (index >= 1 && index < KQT_ARPEGGIO_NOTES_MAX + 1)
    {
        int64_t note = *(int64_t*)data;
        Event_check_integral_range(note, event->field_types[index]);
        arpeggio->notes[index - 1] = note;
        return true;
    }
    return false;
}


static void* Event_voice_arpeggio_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_ARPEGGIO);
    Event_voice_arpeggio* arpeggio = (Event_voice_arpeggio*)event;
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


static void Event_voice_arpeggio_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_ARPEGGIO);
    assert(voice != NULL);
    Event_voice_arpeggio* arpeggio = (Event_voice_arpeggio*)event;
    if (voice->gen->ins_params->scale == NULL ||
            *voice->gen->ins_params->scale == NULL ||
            **voice->gen->ins_params->scale == NULL)
    {
        voice->state.generic.arpeggio = false;
        return;
    }
    Scale* scale = **voice->gen->ins_params->scale;
    int note_count = Scale_get_note_count(scale);
    int orig_note = voice->state.generic.orig_note;
    int orig_note_mod = voice->state.generic.orig_note_mod;
    int orig_octave = voice->state.generic.orig_octave;
    pitch_t orig_pitch = Scale_get_pitch(scale, orig_note, orig_note_mod, orig_octave);
    if (orig_pitch <= 0)
    {
        voice->state.generic.arpeggio = false;
        return;
    }
    int last_nonzero = -1;
    for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i)
    {
        if (arpeggio->notes[i] != 0)
        {
            last_nonzero = i;
        }
        int new_note = orig_note + arpeggio->notes[i];
        int new_octave = orig_octave + (new_note / note_count);
        new_note %= note_count;
        pitch_t new_pitch = Scale_get_pitch(scale, new_note, orig_note_mod, new_octave);
        if (new_pitch <= 0)
        {
            voice->state.generic.arpeggio = false;
            return;
        }
        voice->state.generic.arpeggio_factors[i] = new_pitch / orig_pitch;
    }
    if (last_nonzero == -1)
    {
        voice->state.generic.arpeggio = false;
        return;
    }
    else if (last_nonzero < KQT_ARPEGGIO_NOTES_MAX - 1)
    {
        voice->state.generic.arpeggio_factors[last_nonzero + 1] = -1;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
            voice->state.generic.tempo,
            voice->state.generic.freq);
    voice->state.generic.arpeggio_length = unit_len / arpeggio->speed;
    voice->state.generic.arpeggio_frames = 0;
    voice->state.generic.arpeggio_note = 0;
    voice->state.generic.arpeggio = true;
    return;
}


