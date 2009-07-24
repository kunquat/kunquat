

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

#include <Event_global_set_tempo.h>

#include <xmemory.h>


static Event_field_desc set_tempo_desc[] =
{
    {
        .type = EVENT_FIELD_TYPE_DOUBLE,
        .range.double_type = { 1, 999 }
    },
    {
        .type = EVENT_FIELD_TYPE_NONE
    }
};


static bool Event_global_set_tempo_set(Event* event, int index, void* data);


static void* Event_global_set_tempo_get(Event* event, int index);


static void del_Event_global_set_tempo(Event* event);


static void Event_global_set_tempo_process(Event_global* event, Playdata* play);


Event* new_Event_global_set_tempo(Reltime* pos)
{
    assert(pos != NULL);
    Event_global_set_tempo* event = xalloc(Event_global_set_tempo);
    if (event == NULL)
    {
        return NULL;
    }
    event->parent.parent.type = EVENT_TYPE_GLOBAL_SET_TEMPO;
    event->parent.parent.field_types = set_tempo_desc;
    event->parent.parent.set = Event_global_set_tempo_set;
    event->parent.parent.get = Event_global_set_tempo_get;
    event->parent.parent.destroy = del_Event_global_set_tempo;
    Reltime_copy(&event->parent.parent.pos, pos);
    event->parent.process = Event_global_set_tempo_process;
    return (Event*)event;
}


static void Event_global_set_tempo_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_TYPE_GLOBAL_SET_TEMPO);
    assert(play != NULL);
    Event_global_set_tempo* set_tempo = (Event_global_set_tempo*)event;
    play->tempo = set_tempo->tempo;
    return;
}


static bool Event_global_set_tempo_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_GLOBAL_SET_TEMPO);
    assert(data != NULL);
    Event_global_set_tempo* set_tempo = (Event_global_set_tempo*)event;
    if (index == 0)
    {
        double* tempo = (double*)data;
        Event_check_double_range(*tempo, event->field_types[0]);
        set_tempo->tempo = *tempo;
        return true;
    }
    return false;
}


static void* Event_global_set_tempo_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_GLOBAL_SET_TEMPO);
    Event_global_set_tempo* set_tempo = (Event_global_set_tempo*)event;
    if (index == 0)
    {
        return &set_tempo->tempo;
    }
    return NULL;
}


static void del_Event_global_set_tempo(Event* event)
{
    assert(event != NULL);
    assert(event->type == EVENT_TYPE_GLOBAL_SET_TEMPO);
    xfree(event);
    return;
}


