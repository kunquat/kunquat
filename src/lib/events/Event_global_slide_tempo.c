

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
#include <Event_global_slide_tempo.h>
#include <Reltime.h>

#include <xmemory.h>


static Event_field_desc slide_tempo_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 1, 999 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_global_slide_tempo_set(Event* event, int index, void* data);

static void* Event_global_slide_tempo_get(Event* event, int index);

static void Event_global_slide_tempo_process(Event_global* event, Playdata* play);


Event* new_Event_global_slide_tempo(Reltime* pos)
{
    assert(pos != NULL);
    Event_global_slide_tempo* event = xalloc(Event_global_slide_tempo);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_GLOBAL_SLIDE_TEMPO,
               slide_tempo_desc,
               Event_global_slide_tempo_set,
               Event_global_slide_tempo_get);
    event->parent.process = Event_global_slide_tempo_process;
    event->target_tempo = 120;
    return (Event*)event;
}


static bool Event_global_slide_tempo_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SLIDE_TEMPO);
    assert(data != NULL);
    Event_global_slide_tempo* slide_tempo = (Event_global_slide_tempo*)event;
    if (index == 0)
    {
        double tempo = *(double*)data;
        Event_check_double_range(tempo, event->field_types[0]);
        slide_tempo->target_tempo = tempo;
        return true;
    }
    return false;
}


static void* Event_global_slide_tempo_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SLIDE_TEMPO);
    Event_global_slide_tempo* slide_tempo = (Event_global_slide_tempo*)event;
    if (index == 0)
    {
        return &slide_tempo->target_tempo;
    }
    return NULL;
}


static void Event_global_slide_tempo_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_TEMPO);
    assert(play != NULL);
    Event_global_slide_tempo* slide_tempo = (Event_global_slide_tempo*)event;
    if (play->tempo == slide_tempo->target_tempo)
    {
        return;
    }
    Reltime_init(&play->tempo_slide_int_left);
    Reltime_copy(&play->tempo_slide_left, &play->tempo_slide_length);
    double rems_total = (double)Reltime_get_beats(&play->tempo_slide_length) * KQT_RELTIME_BEAT
                         + Reltime_get_rem(&play->tempo_slide_length);
    double slices = rems_total / 36756720; // slide updated 24 times per beat
    play->tempo_slide_update = (slide_tempo->target_tempo - play->tempo) / slices;
    play->tempo_slide_target = slide_tempo->target_tempo;
    if (play->tempo_slide_update < 0)
    {
        play->tempo_slide = -1;
    }
    else if (play->tempo_slide_update > 0)
    {
        play->tempo_slide = 1;
    }
    else
    {
        play->tempo_slide = 0;
        play->tempo = slide_tempo->target_tempo;
    }
    return;
}


