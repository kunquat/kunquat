

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

#include <Event_common.h>
#include <Event_voice_note_on.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc note_on_desc[] =
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


static bool Event_voice_note_on_set(Event* event, int index, void* data);

static void* Event_voice_note_on_get(Event* event, int index);

static void Event_voice_note_on_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_note_on,
                         EVENT_VOICE_NOTE_ON,
                         note_on_desc,
                         event->note = 0,
                         event->mod = -1,
                         event->octave = KQT_SCALE_MIDDLE_OCTAVE)


static void Event_voice_note_on_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_NOTE_ON);
    assert(voice != NULL);
    Event_voice_note_on* note_on = (Event_voice_note_on*)event;
    Generator_process_note(voice->gen,
                           &voice->state.generic,
                           note_on->note,
                           note_on->mod,
                           note_on->octave);
    voice->state.generic.orig_note = note_on->note;
    voice->state.generic.orig_note_mod = note_on->mod;
    voice->state.generic.orig_octave = note_on->octave;

    voice->state.generic.pedal = &voice->gen->ins_params->pedal;

    Channel_state* ch_state = voice->state.generic.cur_ch_state;

    Reltime_copy(&voice->state.generic.force_slide_length, &ch_state->force_slide_length);
    voice->state.generic.tremolo_length = ch_state->tremolo_length;
    voice->state.generic.tremolo_update = ch_state->tremolo_update;
    voice->state.generic.tremolo_depth_target = ch_state->tremolo_depth;
    voice->state.generic.tremolo_delay_update = ch_state->tremolo_delay_update;

    Reltime_copy(&voice->state.generic.pitch_slide_length, &ch_state->pitch_slide_length);    
    voice->state.generic.vibrato_length = ch_state->vibrato_length;
    voice->state.generic.vibrato_update = ch_state->vibrato_update;
    voice->state.generic.vibrato_depth_target = ch_state->vibrato_depth;
    voice->state.generic.vibrato_delay_update = ch_state->vibrato_delay_update;

    voice->state.generic.panning = ch_state->panning;
    voice->state.generic.panning_slide = ch_state->panning_slide;
    Reltime_copy(&voice->state.generic.panning_slide_length, &ch_state->panning_slide_length);
    voice->state.generic.panning_slide_target = ch_state->panning_slide_target;
    voice->state.generic.panning_slide_frames = ch_state->panning_slide_frames;
    voice->state.generic.panning_slide_update = ch_state->panning_slide_update;

    Reltime_copy(&voice->state.generic.filter_slide_length, &ch_state->filter_slide_length);

    return;
}


static bool Event_voice_note_on_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_NOTE_ON);
    assert(data != NULL);
    Event_voice_note_on* note_on = (Event_voice_note_on*)event;
    int64_t num = *(int64_t*)data;
    if (index < 0 || index >= 3)
    {
        return false;
    }
    Event_check_integral_range(num, event->field_types[index]);
    switch (index)
    {
        case 0:
        {
            note_on->note = num;
        }
        break;
        case 1:
        {
            note_on->mod = num;
        }
        break;
        case 2:
        {
            note_on->octave = num;
        }
        break;
        default:
        {
            assert(false);
        }
        break;
    }
    return true;
}


static void* Event_voice_note_on_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_NOTE_ON);
    Event_voice_note_on* note_on = (Event_voice_note_on*)event;
    switch (index)
    {
        case 0:
        {
            return &note_on->note;
        }
        break;
        case 1:
        {
            return &note_on->mod;
        }
        break;
        case 2:
        {
            return &note_on->octave;
        }
        break;
        default:
        break;
    }
    return NULL;
}


