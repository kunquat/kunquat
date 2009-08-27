

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
#include <math.h>

#include <Event_common.h>
#include <Event_voice_set_filter.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_filter_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -INFINITY, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_primitive_and_get(Event_voice_set_filter,
                             EVENT_VOICE_SET_FILTER,
                             double, cutoff)


static void Event_voice_set_filter_process(Event_voice* event, Voice* voice);


create_constructor(Event_voice_set_filter,
                   EVENT_VOICE_SET_FILTER,
                   set_filter_desc,
                   event->cutoff = INFINITY)


static void Event_voice_set_filter_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FILTER);
    assert(voice != NULL);
    Event_voice_set_filter* set_filter = (Event_voice_set_filter*)event;
    if (set_filter->cutoff > 86)
    {
        voice->state.generic.filter = INFINITY;
        return;
    }
    voice->state.generic.filter = exp2((set_filter->cutoff + 86) / 12);
    return;
}


