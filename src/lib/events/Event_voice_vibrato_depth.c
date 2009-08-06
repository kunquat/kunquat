

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
#include <Event_voice_vibrato_depth.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc vibrato_depth_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_vibrato_depth_set(Event* event, int index, void* data);

static void* Event_voice_vibrato_depth_get(Event* event, int index);

static void Event_voice_vibrato_depth_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_vibrato_depth(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_vibrato_depth* event = xalloc(Event_voice_vibrato_depth);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_VIBRATO_DEPTH,
               vibrato_depth_desc,
               Event_voice_vibrato_depth_set,
               Event_voice_vibrato_depth_get);
    event->parent.process = Event_voice_vibrato_depth_process;
    event->depth = 0;
    return (Event*)event;
}


static bool Event_voice_vibrato_depth_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_VIBRATO_DEPTH);
    assert(data != NULL);
    Event_voice_vibrato_depth* vibrato_depth = (Event_voice_vibrato_depth*)event;
    if (index != 0)
    {
        return false;
    }
    double depth = *(double*)data;
    Event_check_double_range(depth, event->field_types[0]);
    vibrato_depth->depth = depth;
    return true;
}


static void* Event_voice_vibrato_depth_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_VIBRATO_DEPTH);
    Event_voice_vibrato_depth* vibrato_depth = (Event_voice_vibrato_depth*)event;
    if (index != 0)
    {
        return false;
    }
    return &vibrato_depth->depth;
}


static void Event_voice_vibrato_depth_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_VIBRATO_DEPTH);
    assert(voice != NULL);
    Event_voice_vibrato_depth* vibrato_depth = (Event_voice_vibrato_depth*)event;
    if (vibrato_depth->depth > 0 && voice->state.generic.vibrato_length > 0)
    {
        voice->state.generic.vibrato = true;
    }
    voice->state.generic.vibrato_depth_target = vibrato_depth->depth / 240; // unit is 5 cents
    voice->state.generic.vibrato_delay_pos = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->vibrato_depth = voice->state.generic.vibrato_depth;
    return;
}


