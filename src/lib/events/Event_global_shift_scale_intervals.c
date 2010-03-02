

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
#include <Event_global_shift_scale_intervals.h>
#include <File_base.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc shift_scale_intervals_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_SCALES_MAX - 1
    },
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


static bool Event_global_shift_scale_intervals_set(Event* event, int index, void* data);

static void* Event_global_shift_scale_intervals_get(Event* event, int index);


Event_create_constructor(Event_global_shift_scale_intervals,
                         EVENT_GLOBAL_SHIFT_SCALE_INTERVALS,
                         shift_scale_intervals_desc,
                         event->scale_index = 0,
                         event->new_ref = -1,
                         event->fixed_point = 0);


bool Event_global_shift_scale_intervals_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[3];
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
    Scale* scale = global_state->scales[data[0].field.integral_type];
    if (scale == NULL ||
            Scale_get_note_count(scale) <= data[1].field.integral_type ||
            Scale_get_note_count(scale) <= data[2].field.integral_type)
    {
        return true;
    }
    Scale_retune(scale, data[1].field.integral_type,
                 data[2].field.integral_type);
    return true;
}


static bool Event_global_shift_scale_intervals_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SHIFT_SCALE_INTERVALS);
    assert(data != NULL);
    Event_global_shift_scale_intervals* si = (Event_global_shift_scale_intervals*)event;
    if (index == 0)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[0]);
        si->scale_index = index;
        return true;
    }
    else if (index == 1)
    {
        int64_t new_ref = *(int64_t*)data;
        Event_check_integral_range(new_ref, event->field_types[1]);
        si->new_ref = new_ref;
        return true;
    }
    else if (index == 2)
    {
        int64_t fixed_point = *(int64_t*)data;
        Event_check_integral_range(fixed_point, event->field_types[2]);
        si->fixed_point = fixed_point;
        return true;
    }
    return false;
}


static void* Event_global_shift_scale_intervals_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_SHIFT_SCALE_INTERVALS);
    Event_global_shift_scale_intervals* si = (Event_global_shift_scale_intervals*)event;
    if (index == 0)
    {
        return &si->scale_index;
    }
    else if (index == 1)
    {
        return &si->new_ref;
    }
    else if (index == 2)
    {
        return &si->fixed_point;
    }
    return NULL;
}


