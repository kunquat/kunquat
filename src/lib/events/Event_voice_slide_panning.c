

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
#include <limits.h>

#include <Event_common.h>
#include <Event_voice_slide_panning.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_panning_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -1, 1 }
    },
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_slide_panning_set(Event* event, int index, void* data);

static void* Event_voice_slide_panning_get(Event* event, int index);

static void Event_voice_slide_panning_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_slide_panning(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_slide_panning* event = xalloc(Event_voice_slide_panning);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_SLIDE_PANNING,
               slide_panning_desc,
               Event_voice_slide_panning_set,
               Event_voice_slide_panning_get);
    event->parent.process = Event_voice_slide_panning_process;
    event->target_panning = 0;
    Reltime_set(&event->length, 0, 0);
    return (Event*)event;
}


static bool Event_voice_slide_panning_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SLIDE_PANNING);
    assert(data != NULL);
    Event_voice_slide_panning* slide_panning = (Event_voice_slide_panning*)event;
    if (index == 0)
    {
        double panning = *(double*)data;
        Event_check_double_range(panning, event->field_types[0]);
        slide_panning->target_panning = panning;
        return true;
    }
    else if (index == 1)
    {
        Reltime* length = (Reltime*)data;
        Event_check_reltime_range(length, event->field_types[1]);
        Reltime_copy(&slide_panning->length, length);
        return true;
    }
    return false;
}


static void* Event_voice_slide_panning_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SLIDE_PANNING);
    Event_voice_slide_panning* slide_panning = (Event_voice_slide_panning*)event;
    if (index == 0)
    {
        return &slide_panning->target_panning;
    }
    else if (index == 1)
    {
        return &slide_panning->length;
    }
    return NULL;
}


static void Event_voice_slide_panning_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_PANNING);
    assert(voice != NULL);
    Event_voice_slide_panning* slide_panning = (Event_voice_slide_panning*)event;
    voice->state.generic.panning_slide_target = slide_panning->target_panning;
    voice->state.generic.panning_slide_frames =
            Reltime_toframes(&slide_panning->length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    if (voice->state.generic.panning_slide_frames == 0)
    {
        voice->state.generic.panning = voice->state.generic.panning_slide_target;
        voice->state.generic.panning_slide = 0;
        ch_state->panning = slide_panning->target_panning;
        ch_state->panning_slide = 0;
        return;
    }
    double diff = voice->state.generic.panning_slide_target -
            voice->state.generic.panning;
    voice->state.generic.panning_slide_update = diff /
            voice->state.generic.panning_slide_frames;
    if (diff > 0)
    {
        voice->state.generic.panning_slide = 1;
        ch_state->panning_slide = 1;
    }
    else if (diff < 0)
    {
        voice->state.generic.panning_slide = -1;
        ch_state->panning_slide = -1;
    }
    else
    {
        voice->state.generic.panning = voice->state.generic.panning_slide_target;
        voice->state.generic.panning_slide = 0;
        ch_state->panning = slide_panning->target_panning;
        ch_state->panning_slide = 0;
    }
    return;
}


