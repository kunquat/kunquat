

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
                                   double, volume_dB)


static void Event_global_set_volume_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_volume,
                         EVENT_GLOBAL_SET_VOLUME,
                         set_volume_desc,
                         event->volume_dB = 0)


static void Event_global_set_volume_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_VOLUME);
    assert(play != NULL);
    Event_global_set_volume* set_volume = (Event_global_set_volume*)event;
    play->volume = exp2(set_volume->volume_dB / 6);
    play->volume_slide = 0;
    return;
}


