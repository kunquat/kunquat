

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


Event_create_set_primitive_and_get(Event_global_set_volume,
                                   EVENT_GLOBAL_SET_VOLUME,
                                   double, volume_dB)


static void Event_global_set_volume_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_volume,
                         EVENT_GLOBAL_SET_VOLUME,
                         set_volume_desc,
                         event->volume_dB = 0)


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


