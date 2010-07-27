

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
#include <Event_global_shift_scale_intervals.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc shift_scale_intervals_desc[] =
{
    {
        .type = EVENT_FIELD_NOTE,
        .min.field.integral_type = -1,
        .max.field.integral_type = KQT_SCALE_NOTES - 1
    },
    {
        .type = EVENT_FIELD_NOTE,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_SCALE_NOTES - 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_global,
                         EVENT_GLOBAL_SHIFT_SCALE_INTERVALS,
                         shift_scale_intervals);


bool Event_global_shift_scale_intervals_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[2];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, shift_scale_intervals_desc, data, state);
    if (state->error)
    {
        return false;
    }
    if (global_state->scales == NULL)
    {
        return true;
    }
    Scale* scale = global_state->scales[global_state->scale];
    if (scale == NULL ||
            Scale_get_note_count(scale) <= data[0].field.integral_type ||
            Scale_get_note_count(scale) <= data[1].field.integral_type)
    {
        return true;
    }
    Scale_retune(scale, data[0].field.integral_type,
                 data[1].field.integral_type);
    return true;
}


