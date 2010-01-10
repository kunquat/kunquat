

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
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


Event_create_set_primitive_and_get(Event_ins_set_pedal,
                                   EVENT_INS_SET_PEDAL,
                                   double, pedal)


static void Event_ins_set_pedal_process(Event_ins* event);


Event_create_constructor(Event_ins_set_pedal,
                         EVENT_INS_SET_PEDAL,
                         set_pedal_desc,
                         event->parent.ins_params = NULL,
                         event->pedal = 0)


static void Event_ins_set_pedal_process(Event_ins* event)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_INS_SET_PEDAL);
    assert(event->ins_params != NULL);
    Event_ins_set_pedal* set_pedal = (Event_ins_set_pedal*)event;
    event->ins_params->pedal = set_pedal->pedal;
    return;
}


