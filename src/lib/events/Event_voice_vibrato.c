

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
#include <Event_voice_vibrato.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc vibrato_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_vibrato_set(Event* event, int index, void* data);

static void* Event_voice_vibrato_get(Event* event, int index);

static void Event_voice_vibrato_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_vibrato(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_vibrato* event = xalloc(Event_voice_vibrato);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_VIBRATO,
               vibrato_desc,
               Event_voice_vibrato_set,
               Event_voice_vibrato_get);
    event->parent.process = Event_voice_vibrato_process;
    event->speed = 0;
    event->depth = 0;
    return (Event*)event;
}


static bool Event_voice_vibrato_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_VIBRATO);
    assert(data != NULL);
    Event_voice_vibrato* vibrato = (Event_voice_vibrato*)event;
    if (index == 0)
    {
        double speed = *(double*)data;
        Event_check_double_range(speed, event->field_types[0]);
        vibrato->speed = speed;
        return true;
    }
    else if (index == 1)
    {
        double depth = *(double*)data;
        Event_check_double_range(depth, event->field_types[1]);
        vibrato->depth = depth;
        return true;
    }
    return false;
}


static void* Event_voice_vibrato_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_VIBRATO);
    Event_voice_vibrato* vibrato = (Event_voice_vibrato*)event;
    if (index == 0)
    {
        return &vibrato->speed;
    }
    else if (index == 1)
    {
        return &vibrato->depth;
    }
    return NULL;
}


static void Event_voice_vibrato_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_VIBRATO);
    assert(voice != NULL);
    Event_voice_vibrato* vibrato = (Event_voice_vibrato*)event;
    if (vibrato->speed > 0 && vibrato->depth > 0)
    {
        voice->state.generic.vibrato = true;
    }
    else
    {
        voice->state.generic.vibrato = false;
    }
    if (vibrato->speed > 0)
    {
        double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                voice->state.generic.tempo,
                voice->state.generic.freq);
        voice->state.generic.vibrato_length = unit_len / vibrato->speed;
        voice->state.generic.vibrato_update = (2 * PI) / voice->state.generic.vibrato_length;
    }
    if (vibrato->depth > 0)
    {
        voice->state.generic.vibrato_depth = vibrato->depth / 240; // unit is 5 cents
    }
    return;
}


