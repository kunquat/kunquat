

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


create_set_primitive_and_get(Event_global_set_scale,
                             EVENT_GLOBAL_SET_SCALE,
                             int64_t, scale_index)


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


static void Event_global_set_scale_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_SCALE);
    assert(play != NULL);
    Event_global_set_scale* set_scale = (Event_global_set_scale*)event;
    if (play->scales == NULL)
    {
        return;
    }
    play->active_scale = &play->scales[set_scale->scale_index];
    return;
}


