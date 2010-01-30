

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
#include <Event_global_retune_scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc retune_scale_desc[] =
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


static bool Event_global_retune_scale_set(Event* event, int index, void* data);

static void* Event_global_retune_scale_get(Event* event, int index);

static void Event_global_retune_scale_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_retune_scale,
                         EVENT_GLOBAL_RETUNE_SCALE,
                         retune_scale_desc,
                         event->scale_index = 0,
                         event->new_ref = -1,
                         event->fixed_point = 0)


#if 0
bool Event_global_retune_scale_handle(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
}
#endif


static void Event_global_retune_scale_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_RETUNE_SCALE);
    assert(play != NULL);
    Event_global_retune_scale* retune_scale = (Event_global_retune_scale*)event;
    if (play->scales == NULL)
    {
        return;
    }
    Scale* scale = play->scales[retune_scale->scale_index];
    if (scale == NULL ||
            Scale_get_note_count(scale) <= retune_scale->new_ref ||
            Scale_get_note_count(scale) <= retune_scale->fixed_point)
    {
        return;
    }
    Scale_retune(scale, retune_scale->new_ref, retune_scale->fixed_point);
    return;
}


static bool Event_global_retune_scale_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_RETUNE_SCALE);
    assert(data != NULL);
    Event_global_retune_scale* retune_scale = (Event_global_retune_scale*)event;
    if (index == 0)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[0]);
        retune_scale->scale_index = index;
        return true;
    }
    else if (index == 1)
    {
        int64_t new_ref = *(int64_t*)data;
        Event_check_integral_range(new_ref, event->field_types[1]);
        retune_scale->new_ref = new_ref;
        return true;
    }
    else if (index == 2)
    {
        int64_t fixed_point = *(int64_t*)data;
        Event_check_integral_range(fixed_point, event->field_types[2]);
        retune_scale->fixed_point = fixed_point;
        return true;
    }
    return false;
}


static void* Event_global_retune_scale_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_GLOBAL_RETUNE_SCALE);
    Event_global_retune_scale* retune_scale = (Event_global_retune_scale*)event;
    if (index == 0)
    {
        return &retune_scale->scale_index;
    }
    else if (index == 1)
    {
        return &retune_scale->new_ref;
    }
    else if (index == 2)
    {
        return &retune_scale->fixed_point;
    }
    return NULL;
}


