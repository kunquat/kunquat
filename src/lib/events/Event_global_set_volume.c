

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
#include <math.h>

#include <Event_common.h>
#include <Event_global_set_volume.h>

#include <xmemory.h>


static Event_field_desc set_volume_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = 0,
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_set_volume,
                                   EVENT_GLOBAL_SET_VOLUME,
                                   double, volume_dB);


Event_create_constructor(Event_global_set_volume,
                         EVENT_GLOBAL_SET_VOLUME,
                         set_volume_desc,
                         event->volume_dB = 0);


bool Event_global_set_volume_handle(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_volume_desc, data, state);
    if (state->error)
    {
        return false;
    }
    global_state->volume = exp2(data[0].field.double_type / 6);
    global_state->volume_slide = 0;
    return true;
}


