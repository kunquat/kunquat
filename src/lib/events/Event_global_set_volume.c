

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
#include <Event_global_set_volume.h>

#include <xmemory.h>


static Event_field_desc set_volume_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -INFINITY, 0 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_global_set_volume_set(Event* event, int index, void* data);

static void* Event_global_set_volume_get(Event* event, int index);

static void Event_global_set_volume_process(Event_global* event, Playdata* play);


Event* new_Event_global_set_volume(Reltime* pos)
{
    assert(pos != NULL);
    Event_global_set_volume* event = xalloc(Event_global_set_volume);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_GLOBAL_SET_VOLUME,
               set_volume_desc,
               Event_global_set_volume_set,
               Event_global_set_volume_get);
    event->parent.process = Event_global_set_volume_process;
    return (Event*)event;
}


static bool Event_global_set_volume_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SET_VOLUME);
    assert(data != NULL);
    Event_global_set_volume* set_volume = (Event_global_set_volume*)event;
    if (index == 0)
    {
        double volume_dB = *(double*)data;
        Event_check_double_range(volume_dB, event->field_types[0]);
        set_volume->volume_dB = volume_dB;
        return true;
    }
    return false;
}


static void* Event_global_set_volume_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SET_VOLUME);
    Event_global_set_volume* set_volume = (Event_global_set_volume*)event;
    if (index == 0)
    {
        return &set_volume->volume_dB;
    }
    return NULL;
}


static void Event_global_set_volume_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_VOLUME);
    assert(play != NULL);
    Event_global_set_volume* set_volume = (Event_global_set_volume*)event;
    play->volume = exp2(set_volume->volume_dB / 6);
    play->volume_slide = 0;
    return;
}


