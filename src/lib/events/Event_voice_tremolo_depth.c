

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
#include <Event_voice_tremolo_depth.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc tremolo_depth_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, 24 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_tremolo_depth,
                                   EVENT_VOICE_TREMOLO_DEPTH,
                                   double, depth)


static void Event_voice_tremolo_depth_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_tremolo_depth,
                         EVENT_VOICE_TREMOLO_DEPTH,
                         tremolo_depth_desc,
                         event->depth = 0)


static void Event_voice_tremolo_depth_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO_DEPTH);
    assert(voice != NULL);
    Event_voice_tremolo_depth* tremolo_depth = (Event_voice_tremolo_depth*)event;
    if (tremolo_depth->depth > 0 && voice->state.generic.tremolo_length > 0)
    {
        voice->state.generic.tremolo = true;
    }
    voice->state.generic.tremolo_depth_target = tremolo_depth->depth;
    voice->state.generic.tremolo_delay_pos = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->tremolo_depth = voice->state.generic.tremolo_depth_target;
    return;
}


