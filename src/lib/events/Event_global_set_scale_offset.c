

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
#include <float.h>

#include <Event_common.h>
#include <Event_global_set_scale_offset.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_scale_offset_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -DBL_MAX,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_global,
                         EVENT_GLOBAL_SET_SCALE_OFFSET,
                         set_scale_offset);


bool Event_global_set_scale_offset_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_scale_offset_desc, data, state);
    if (state->error)
    {
        return false;
    }
    if (global_state->scales == NULL)
    {
        return true;
    }
    Scale* scale = global_state->scales[global_state->scale];
    if (scale == NULL)
    {
        return true;
    }
    Scale_set_pitch_offset(scale, data[0].field.double_type);
    return true;
}


