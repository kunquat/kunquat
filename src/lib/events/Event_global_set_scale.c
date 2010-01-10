

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
#include <Event_global_set_scale.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_scale_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 0, KQT_SCALES_MAX - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_set_scale,
                                   EVENT_GLOBAL_SET_SCALE,
                                   int64_t, scale_index)


static void Event_global_set_scale_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_scale,
                         EVENT_GLOBAL_SET_SCALE,
                         set_scale_desc,
                         event->scale_index = 0)


static void Event_global_set_scale_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_SCALE);
    assert(play != NULL);
    Event_global_set_scale* set_scale = (Event_global_set_scale*)event;
    if (play->scales == NULL)
    {
        return;
    }
    play->active_scale = &play->scales[set_scale->scale_index];
    return;
}


