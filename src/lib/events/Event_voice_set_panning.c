

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
#include <Event_voice_set_panning.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_panning_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -1, 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_set_panning_set(Event* event, int index, void* data);

static void* Event_voice_set_panning_get(Event* event, int index);

static void Event_voice_set_panning_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_set_panning(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_set_panning* event = xalloc(Event_voice_set_panning);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_SET_PANNING,
               set_panning_desc,
               Event_voice_set_panning_set,
               Event_voice_set_panning_get);
    event->parent.process = Event_voice_set_panning_process;
    event->panning = 0;
    return (Event*)event;
}


static bool Event_voice_set_panning_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_PANNING);
    assert(data != NULL);
    Event_voice_set_panning* set_panning = (Event_voice_set_panning*)event;
    if (index != 0)
    {
        return false;
    }
    double panning = *(double*)data;
    Event_check_double_range(panning, event->field_types[0]);
    set_panning->panning = panning;
    return true;
}


static void* Event_voice_set_panning_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_PANNING);
    Event_voice_set_panning* set_panning = (Event_voice_set_panning*)event;
    if (index != 0)
    {
        return NULL;
    }
    return &set_panning->panning;
}


static void Event_voice_set_panning_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_PANNING);
    assert(voice != NULL);
    Event_voice_set_panning* set_panning = (Event_voice_set_panning*)event;
    voice->state.generic.panning = set_panning->panning;
    voice->state.generic.panning_slide = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->panning = set_panning->panning;
    ch_state->panning_slide = 0;
    return;
}


