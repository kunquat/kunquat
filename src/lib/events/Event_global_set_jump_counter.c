

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
#include <Event_global_set_jump_counter.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_jump_counter_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 0, 65535 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_primitive_and_get(Event_global_set_jump_counter,
                             EVENT_GLOBAL_SET_JUMP_COUNTER,
                             int64_t, counter)


static void Event_global_set_jump_counter_process(Event_global* event, Playdata* play);


create_constructor(Event_global_set_jump_counter,
                   EVENT_GLOBAL_SET_JUMP_COUNTER,
                   set_jump_counter_desc,
                   event->counter = 0)


static void Event_global_set_jump_counter_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_JUMP_COUNTER);
    assert(play != NULL);
    Event_global_set_jump_counter* set_jump_counter = (Event_global_set_jump_counter*)event;
    play->jump_set_counter = set_jump_counter->counter;
    return;
}


