

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
#include <Event_global_mimic_scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc mimic_scale_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_SCALES_MAX - 1
    },
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_SCALES_MAX - 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_global_mimic_scale_set(Event* event, int index, void* data);

static void* Event_global_mimic_scale_get(Event* event, int index);


Event_create_constructor(Event_global_mimic_scale,
                         EVENT_GLOBAL_MIMIC_SCALE,
                         mimic_scale_desc,
                         event->scale_index = 0,
                         event->modifier_index = 0);


bool Event_global_mimic_scale_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[2];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, mimic_scale_desc, data, state);
    if (state->error)
    {
        return false;
    }
    if (global_state->scales == NULL)
    {
        return true;
    }
    Scale* scale = global_state->scales[data[0].field.integral_type];
    Scale* modifier = global_state->scales[data[1].field.integral_type];
    if (scale == NULL || modifier == NULL)
    {
        return true;
    }
    Scale_retune_with_source(scale, modifier);
    return true;
}


static bool Event_global_mimic_scale_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_MIMIC_SCALE);
    assert(data != NULL);
    Event_global_mimic_scale* ms = (Event_global_mimic_scale*)event;
    if (index == 0)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[0]);
        ms->scale_index = index;
        return true;
    }
    else if (index == 1)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[1]);
        ms->modifier_index = index;
        return true;
    }
    return false;
}


static void* Event_global_mimic_scale_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_MIMIC_SCALE);
    Event_global_mimic_scale* ms = (Event_global_mimic_scale*)event;
    if (index == 0)
    {
        return &ms->scale_index;
    }
    else if (index == 1)
    {
        return &ms->modifier_index;
    }
    return NULL;
}


