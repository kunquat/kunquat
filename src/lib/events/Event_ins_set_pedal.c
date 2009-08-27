

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
#include <Event_ins_set_pedal.h>
#include <Instrument_params.h>

#include <xmemory.h>


static Event_field_desc set_pedal_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_primitive_and_get(Event_ins_set_pedal,
                             EVENT_INS_SET_PEDAL,
                             double, pedal)


static void Event_ins_set_pedal_process(Event_ins* event);


Event* new_Event_ins_set_pedal(Reltime* pos)
{
    assert(pos != NULL);
    Event_ins_set_pedal* event = xalloc(Event_ins_set_pedal);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_INS_SET_PEDAL,
               set_pedal_desc,
               Event_ins_set_pedal_set,
               Event_ins_set_pedal_get);
    event->parent.process = Event_ins_set_pedal_process;
    event->parent.ins_params = NULL;
    event->pedal = 0;
    return (Event*)event;
}


static void Event_ins_set_pedal_process(Event_ins* event)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_INS_SET_PEDAL);
    assert(event->ins_params != NULL);
    Event_ins_set_pedal* set_pedal = (Event_ins_set_pedal*)event;
    event->ins_params->pedal = set_pedal->pedal;
    return;
}


