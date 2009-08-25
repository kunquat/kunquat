

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
#include <Event_channel_set_instrument.h>
#include <Channel.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_instrument_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 0, KQT_INSTRUMENTS_MAX }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_channel_set_instrument_set(Event* event, int index, void* data);

static void* Event_channel_set_instrument_get(Event* event, int index);

static void Event_channel_set_instrument_process(Event_channel* event, Channel* ch);


Event* new_Event_channel_set_instrument(Reltime* pos)
{
    assert(pos != NULL);
    Event_channel_set_instrument* event = xalloc(Event_channel_set_instrument);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_CHANNEL_SET_INSTRUMENT,
               set_instrument_desc,
               Event_channel_set_instrument_set,
               Event_channel_set_instrument_get);
    event->parent.process = Event_channel_set_instrument_process;
    event->instrument = 0;
    return (Event*)event;
}


static bool Event_channel_set_instrument_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_SET_INSTRUMENT);
    assert(data != NULL);
    Event_channel_set_instrument* set_instrument = (Event_channel_set_instrument*)event;
    if (index == 0)
    {
        int64_t instrument = *(int64_t*)data;
        Event_check_integral_range(instrument, event->field_types[0]);
        set_instrument->instrument = instrument;
        return true;
    }
    return false;
}


static void* Event_channel_set_instrument_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_CHANNEL_SET_INSTRUMENT);
    Event_channel_set_instrument* set_instrument = (Event_channel_set_instrument*)event;
    if (index == 0)
    {
        return &set_instrument->instrument;
    }
    return NULL;
}


static void Event_channel_set_instrument_process(Event_channel* event, Channel* ch)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_CHANNEL_SET_INSTRUMENT);
    assert(ch != NULL);
    Event_channel_set_instrument* set_instrument = (Event_channel_set_instrument*)event;
    ch->cur_inst = set_instrument->instrument;
    return;
}


