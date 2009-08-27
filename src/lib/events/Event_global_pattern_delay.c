

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
#include <Event_global_pattern_delay.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc pattern_delay_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_reltime_and_get(Event_global_pattern_delay,
                           EVENT_GLOBAL_PATTERN_DELAY,
                           length)


static void Event_global_pattern_delay_process(Event_global* event, Playdata* play);


create_constructor(Event_global_pattern_delay,
                   EVENT_GLOBAL_PATTERN_DELAY,
                   pattern_delay_desc,
                   Reltime_init(&event->length))


static void Event_global_pattern_delay_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_PATTERN_DELAY);
    assert(play != NULL);
    Event_global_pattern_delay* pattern_delay = (Event_global_pattern_delay*)event;
    Reltime_copy(&play->delay_left, &pattern_delay->length);
    return;
}


