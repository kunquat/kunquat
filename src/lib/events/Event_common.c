

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

#include <Event_common.h>

#include <xmemory.h>


Event* Event_init(Event* event,
                  Reltime* pos,
                  Event_type type,
                  Event_field_desc* field_types,
                  bool (*set)(Event*, int, void*),
                  void* (*get)(Event*, int))
{
    assert(event != NULL);
    assert(pos != NULL);
    assert(EVENT_IS_VALID(type));
    assert(field_types != NULL);
    assert(set != NULL);
    assert(get != NULL);
    event->type = type;
    Reltime_copy(&event->pos, pos);
    event->field_types = field_types;
    event->set = set;
    event->get = get;
    event->destroy = del_Event_default;
    return event;
}


void del_Event_default(Event* event)
{
    assert(event != NULL);
    assert(EVENT_IS_VALID(event->type));
    xfree(event);
    return;
}


