

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
#include <Event_voice_tremolo.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc tremolo_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, 24 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_tremolo_set(Event* event, int index, void* data);

static void* Event_voice_tremolo_get(Event* event, int index);

static void Event_voice_tremolo_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_tremolo(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_tremolo* event = xalloc(Event_voice_tremolo);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_TREMOLO,
               tremolo_desc,
               Event_voice_tremolo_set,
               Event_voice_tremolo_get);
    event->parent.process = Event_voice_tremolo_process;
    event->speed = 0;
    event->depth = 0;
    return (Event*)event;
}


static bool Event_voice_tremolo_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_TREMOLO);
    assert(data != NULL);
    Event_voice_tremolo* tremolo = (Event_voice_tremolo*)event;
    if (index == 0)
    {
        double speed = *(double*)data;
        Event_check_double_range(speed, event->field_types[0]);
        tremolo->speed = speed;
        return true;
    }
    else if (index == 1)
    {
        double depth = *(double*)data;
        Event_check_double_range(depth, event->field_types[1]);
        tremolo->depth = depth;
        return true;
    }
    return false;
}


static void* Event_voice_tremolo_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_TREMOLO);
    Event_voice_tremolo* tremolo = (Event_voice_tremolo*)event;
    if (index == 0)
    {
        return &tremolo->speed;
    }
    else if (index == 1)
    {
        return &tremolo->depth;
    }
    return NULL;
}


static void Event_voice_tremolo_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO);
    assert(voice != NULL);
    Event_voice_tremolo* tremolo = (Event_voice_tremolo*)event;
    if (tremolo->speed > 0 && tremolo->depth > 0)
    {
        voice->state.generic.tremolo = true;
    }
    else
    {
        voice->state.generic.tremolo = false;
    }
    if (tremolo->speed > 0)
    {
        double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                voice->state.generic.tempo,
                voice->state.generic.freq);
        voice->state.generic.tremolo_length = unit_len / tremolo->speed;
        voice->state.generic.tremolo_update = (2 * PI) / voice->state.generic.tremolo_length;
    }
    if (tremolo->depth > 0)
    {
        voice->state.generic.tremolo_depth = tremolo->depth;
    }
    return;
}


