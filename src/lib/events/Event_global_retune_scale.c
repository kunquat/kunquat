

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
#include <Event_global_retune_scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc retune_scale_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 0, KQT_SCALES_MAX - 1 }
    },
    {
        .type = EVENT_FIELD_NOTE,
        .range.integral_type = { -1, KQT_SCALE_NOTES - 1 }
    },
    {
        .type = EVENT_FIELD_NOTE,
        .range.integral_type = { 0, KQT_SCALE_NOTES - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_global_retune_scale_set(Event* event, int index, void* data);

static void* Event_global_retune_scale_get(Event* event, int index);

static void Event_global_retune_scale_process(Event_global* event, Playdata* play);


create_constructor(Event_global_retune_scale,
                   EVENT_GLOBAL_RETUNE_SCALE,
                   retune_scale_desc,
                   event->scale_index = 0,
                   event->new_ref = -1,
                   event->fixed_point = 0)


static void Event_global_retune_scale_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_RETUNE_SCALE);
    assert(play != NULL);
    Event_global_retune_scale* retune_scale = (Event_global_retune_scale*)event;
    if (play->scales == NULL)
    {
        return;
    }
    Scale* scale = play->scales[retune_scale->scale_index];
    if (scale == NULL ||
            Scale_get_note_count(scale) <= retune_scale->new_ref ||
            Scale_get_note_count(scale) <= retune_scale->fixed_point)
    {
        return;
    }
    Scale_retune(scale, retune_scale->new_ref, retune_scale->fixed_point);
    return;
}


static bool Event_global_retune_scale_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_RETUNE_SCALE);
    assert(data != NULL);
    Event_global_retune_scale* retune_scale = (Event_global_retune_scale*)event;
    if (index == 0)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[0]);
        retune_scale->scale_index = index;
        return true;
    }
    else if (index == 1)
    {
        int64_t new_ref = *(int64_t*)data;
        Event_check_integral_range(new_ref, event->field_types[1]);
        retune_scale->new_ref = new_ref;
        return true;
    }
    else if (index == 2)
    {
        int64_t fixed_point = *(int64_t*)data;
        Event_check_integral_range(fixed_point, event->field_types[2]);
        retune_scale->fixed_point = fixed_point;
        return true;
    }
    return false;
}


static void* Event_global_retune_scale_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_RETUNE_SCALE);
    Event_global_retune_scale* retune_scale = (Event_global_retune_scale*)event;
    if (index == 0)
    {
        return &retune_scale->scale_index;
    }
    else if (index == 1)
    {
        return &retune_scale->new_ref;
    }
    else if (index == 2)
    {
        return &retune_scale->fixed_point;
    }
    return NULL;
}


