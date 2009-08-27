

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
#include <limits.h>

#include <Event_common.h>
#include <Event_voice_slide_force.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_force_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -INFINITY, 18 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_primitive_and_get(Event_voice_slide_force,
                             EVENT_VOICE_SLIDE_FORCE,
                             double, target_force)


static void Event_voice_slide_force_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_slide_force(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_slide_force* event = xalloc(Event_voice_slide_force);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_SLIDE_FORCE,
               slide_force_desc,
               Event_voice_slide_force_set,
               Event_voice_slide_force_get);
    event->parent.process = Event_voice_slide_force_process;
    event->target_force = 1;
    return (Event*)event;
}


static void Event_voice_slide_force_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_FORCE);
    assert(voice != NULL);
    assert(voice->state.generic.tempo > 0);
    assert(voice->state.generic.freq > 0);
    Event_voice_slide_force* slide_force = (Event_voice_slide_force*)event;
    voice->state.generic.force_slide_target = exp2(slide_force->target_force / 6);
    voice->state.generic.force_slide_frames =
            Reltime_toframes(&voice->state.generic.force_slide_length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    double force_dB = log2(voice->state.generic.force) * 6;
    double dB_step = (slide_force->target_force - force_dB) /
            voice->state.generic.force_slide_frames;
    voice->state.generic.force_slide_update = exp2(dB_step / 6);
    if (dB_step > 0)
    {
        voice->state.generic.force_slide = 1;
    }
    else if (dB_step < 0)
    {
        voice->state.generic.force_slide = -1;
    }
    else
    {
        voice->state.generic.force = voice->state.generic.force_slide_target;
    }
    return;
}


