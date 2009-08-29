

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
#include <Event_global_set_tempo.h>

#include <xmemory.h>


static Event_field_desc set_tempo_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 1, 999 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_set_tempo,
                                   EVENT_GLOBAL_SET_TEMPO,
                                   double, tempo)


static void Event_global_set_tempo_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_tempo,
                         EVENT_GLOBAL_SET_TEMPO,
                         set_tempo_desc,
                         event->tempo = 120)


static void Event_global_set_tempo_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_TEMPO);
    assert(play != NULL);
    Event_global_set_tempo* set_tempo = (Event_global_set_tempo*)event;
    play->tempo = set_tempo->tempo;
    play->tempo_slide = 0;
    return;
}


