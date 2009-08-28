

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


Event_create_set_primitive_and_get(Event_channel_set_instrument,
                                   EVENT_CHANNEL_SET_INSTRUMENT,
                                   int64_t, instrument)


static void Event_channel_set_instrument_process(Event_channel* event, Channel* ch);


Event_create_constructor(Event_channel_set_instrument,
                         EVENT_CHANNEL_SET_INSTRUMENT,
                         set_instrument_desc,
                         event->instrument = 0)


static void Event_channel_set_instrument_process(Event_channel* event, Channel* ch)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_CHANNEL_SET_INSTRUMENT);
    assert(ch != NULL);
    Event_channel_set_instrument* set_instrument = (Event_channel_set_instrument*)event;
    ch->cur_inst = set_instrument->instrument;
    return;
}


