

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
#include <stdbool.h>

#include <Event_common.h>
#include <Event_ins_set_pedal.h>
#include <Instrument_params.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_pedal_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = 0,
        .max.field.double_type = 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_ins_set_pedal,
                                   EVENT_INS_SET_PEDAL,
                                   double, pedal);


Event_create_constructor(Event_ins_set_pedal,
                         EVENT_INS_SET_PEDAL,
                         set_pedal_desc,
                         event->pedal = 0);


bool Event_ins_set_pedal_process(Instrument_params* ins_state, char* fields)
{
    assert(ins_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_pedal_desc, data, state);
    if (state->error)
    {
        return false;
    }
    ins_state->pedal = data[0].field.double_type;
    return true;
}


