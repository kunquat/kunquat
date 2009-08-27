

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

#include <Event_common.h>
#include <Event_global_slide_tempo_length.h>
#include <Reltime.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_tempo_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_reltime_and_get(Event_global_slide_tempo_length,
                           EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                           length)


static void Event_global_slide_tempo_length_process(Event_global* event, Playdata* play);


Event* new_Event_global_slide_tempo_length(Reltime* pos)
{
    assert(pos != NULL);
    Event_global_slide_tempo_length* event = xalloc(Event_global_slide_tempo_length);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
               slide_tempo_length_desc,
               Event_global_slide_tempo_length_set,
               Event_global_slide_tempo_length_get);
    event->parent.process = Event_global_slide_tempo_length_process;
    Reltime_set(&event->length, 0, 0);
    return (Event*)event;
}


static void Event_global_slide_tempo_length_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_TEMPO_LENGTH);
    assert(play != NULL);
    Event_global_slide_tempo_length* slide_tempo_length = (Event_global_slide_tempo_length*)event;
    if (play->tempo_slide != 0)
    {
        Reltime_init(&play->tempo_slide_int_left);
        Reltime_copy(&play->tempo_slide_left, &slide_tempo_length->length);
        double rems_total = (double)Reltime_get_beats(&slide_tempo_length->length) *
                            KQT_RELTIME_BEAT +
                            Reltime_get_rem(&slide_tempo_length->length);
        double slices = rems_total / 36756720; // slide updated 24 times per beat
        play->tempo_slide_update = (play->tempo_slide_target - play->tempo) / slices;
    }
    Reltime_copy(&play->tempo_slide_length, &slide_tempo_length->length);
    return;
}


