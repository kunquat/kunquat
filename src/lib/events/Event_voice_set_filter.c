

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


static bool Event_voice_set_filter_set(Event* event, int index, void* data);

static void* Event_voice_set_filter_get(Event* event, int index);

static void Event_voice_set_filter_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_set_filter(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_set_filter* event = xalloc(Event_voice_set_filter);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_SET_FILTER,
               set_filter_desc,
               Event_voice_set_filter_set,
               Event_voice_set_filter_get);
    event->parent.process = Event_voice_set_filter_process;
    event->cutoff = INFINITY;
    return (Event*)event;
}


static bool Event_voice_set_filter_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_FILTER);
    assert(data != NULL);
    Event_voice_set_filter* set_filter = (Event_voice_set_filter*)event;
    if (index != 0)
    {
        return false;
    }
    double cutoff = *(double*)data;
    Event_check_double_range(cutoff, event->field_types[0]);
    set_filter->cutoff = cutoff;
    return true;
}


static void* Event_voice_set_filter_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_SET_FILTER);
    Event_voice_set_filter* set_filter = (Event_voice_set_filter*)event;
    if (index != 0)
    {
        return false;
    }
    return &set_filter->cutoff;
}


static void Event_voice_set_filter_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FILTER);
    assert(voice != NULL);
    Event_voice_set_filter* set_filter = (Event_voice_set_filter*)event;
    if (isinf(set_filter->cutoff))
    {
        voice->state.generic.actual_filter = voice->state.generic.filter = INFINITY;
        voice->state.generic.filter_update = true;
        return;
    }
    voice->state.generic.filter = exp2((set_filter->cutoff + 86) / 12);
    if (voice->state.generic.filter != voice->state.generic.actual_filter)
    {
        voice->state.generic.filter_update = true;
    }
    return;
}


