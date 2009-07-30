

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

#include <Event_voice_set_force.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_force_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -INFINITY, INFINITY }
    }
};


static bool Event_voice_set_force_set(Event* event, int index, void* data);

static void* Event_voice_set_force_get(Event* event, int index);

static void del_Event_voice_set_force(Event* event);

static void Event_voice_set_force_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_set_force(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_set_force* event = xalloc(Event_voice_set_force);
    if (event == NULL)
    {
        return NULL;
    }
    Reltime_copy(&event->parent.parent.pos, pos);
    event->parent.parent.type = EVENT_VOICE_SET_FORCE;
    event->parent.parent.field_types = set_force_desc;
    event->parent.parent.set = Event_voice_set_force_set;
    event->parent.parent.get = Event_voice_set_force_get;
    event->parent.parent.destroy = del_Event_voice_set_force;
    event->parent.process = Event_voice_set_force_process;
    event->force = 0;
    return (Event*)event;
}


static bool Event_voice_set_force_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_FORCE);
    assert(data != NULL);
    Event_voice_set_force* set_force = (Event_voice_set_force*)event;
    if (index != 0)
    {
        return false;
    }
    double* num = (double*)data;
    if (isnan(*num))
    {
        return false;
    }
    set_force->force = *num;
    return true;
}


static void* Event_voice_set_force_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_FORCE);
    Event_voice_set_force* set_force = (Event_voice_set_force*)event;
    if (index != 0)
    {
        return NULL;
    }
    return &set_force->force;
}


static void Event_voice_set_force_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FORCE);
    assert(voice != NULL);
    Event_voice_set_force* set_force = (Event_voice_set_force*)event;
    voice->state.generic.force = exp2(set_force->force / 6);
    return;
}


static void del_Event_voice_set_force(Event* event)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_FORCE);
    xfree(event);
    return;
}


