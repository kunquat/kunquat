

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
#include <Event_global_set_scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_scale_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 0, KQT_SCALES_MAX - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_global_set_scale_set(Event* event, int index, void* data);

static void* Event_global_set_scale_get(Event* event, int index);

static void Event_global_set_scale_process(Event_global* event, Playdata* play);


Event* new_Event_global_set_scale(Reltime* pos)
{
    assert(pos != NULL);
    Event_global_set_scale* event = xalloc(Event_global_set_scale);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_GLOBAL_SET_SCALE,
               set_scale_desc,
               Event_global_set_scale_set,
               Event_global_set_scale_get);
    event->parent.process = Event_global_set_scale_process;
    return (Event*)event;
}


static bool Event_global_set_scale_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SET_SCALE);
    assert(data != NULL);
    Event_global_set_scale* set_scale = (Event_global_set_scale*)event;
    if (index == 0)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[0]);
        set_scale->scale_index = index;
        return true;
    }
    return false;
}


static void* Event_global_set_scale_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SET_SCALE);
    Event_global_set_scale* set_scale = (Event_global_set_scale*)event;
    if (index == 0)
    {
        return &set_scale->scale_index;
    }
    return NULL;
}


static void Event_global_set_scale_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_SCALE);
    assert(play != NULL);
    Event_global_set_scale* set_scale = (Event_global_set_scale*)event;
    play->active_scale = &play->scales[set_scale->scale_index];
    return;
}


