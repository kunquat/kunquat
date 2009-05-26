

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

#include <Event.h>

#include <Note_table.h>
#include <Song_limits.h>

#include <xmemory.h>


Event_field_desc* Event_get_field_types(Event* event)
{
    assert(event != NULL);
    assert(event->field_types != NULL);
    return event->field_types;
}


Reltime* Event_get_pos(Event* event)
{
    assert(event != NULL);
    return &event->pos;
}


void Event_set_pos(Event* event, Reltime* pos)
{
    assert(event != NULL);
    assert(pos != NULL);
    Reltime_copy(&event->pos, pos);
    return;
}


Event_type Event_get_type(Event* event)
{
    assert(event != NULL);
    return event->type;
}


void* Event_get_field(Event* event, int index)
{
    assert(event != NULL);
    assert(event->get != NULL);
    return event->get(event, index);
}


bool Event_set_field(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->set != NULL);
    assert(data != NULL);
    return event->set(event, index, data);
}


void del_Event(Event* event)
{
    assert(event != NULL);
    assert(event->destroy != NULL);
    event->destroy(event);
    return;
}


