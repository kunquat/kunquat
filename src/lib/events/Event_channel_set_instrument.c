

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
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


