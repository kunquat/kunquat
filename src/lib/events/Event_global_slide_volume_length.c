

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
#include <limits.h>
#include <math.h>

#include <Event_common.h>
#include <Event_global_slide_volume_length.h>
#include <Reltime.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_volume_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_reltime_and_get(Event_global_slide_volume_length,
                           EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                           length)


static void Event_global_slide_volume_length_process(Event_global* event, Playdata* play);


Event* new_Event_global_slide_volume_length(Reltime* pos)
{
    assert(pos != NULL);
    Event_global_slide_volume_length* event = xalloc(Event_global_slide_volume_length);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
               slide_volume_length_desc,
               Event_global_slide_volume_length_set,
               Event_global_slide_volume_length_get);
    event->parent.process = Event_global_slide_volume_length_process;
    Reltime_set(&event->length, 0, 0);
    return (Event*)event;
}


static void Event_global_slide_volume_length_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_VOLUME_LENGTH);
    assert(play != NULL);
    Event_global_slide_volume_length* slide_volume_length = (Event_global_slide_volume_length*)event;
    if (play->volume_slide != 0)
    {
        play->volume_slide_frames = Reltime_toframes(&slide_volume_length->length,
                                                     play->tempo,
                                                     play->freq);
        double volume_dB = log2(play->volume) * 6;
        double target_dB = log2(play->volume_slide_target) * 6;
        double dB_step = (target_dB - volume_dB) / play->volume_slide_frames;
        play->volume_slide_update = exp2(dB_step / 6);
    }
    Reltime_copy(&play->volume_slide_length, &slide_volume_length->length);
    return;
}


