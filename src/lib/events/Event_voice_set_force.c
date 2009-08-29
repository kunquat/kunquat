

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
#include <Event_voice_set_force.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_force_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -INFINITY, 18 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_set_force,
                                   EVENT_VOICE_SET_FORCE,
                                   double, force_dB)


static void Event_voice_set_force_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_set_force,
                         EVENT_VOICE_SET_FORCE,
                         set_force_desc,
                         event->force_dB = 0)


static void Event_voice_set_force_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FORCE);
    assert(voice != NULL);
    Event_voice_set_force* set_force = (Event_voice_set_force*)event;
    voice->state.generic.force = exp2(set_force->force_dB / 6);
    voice->state.generic.force_slide = 0;
    return;
}


