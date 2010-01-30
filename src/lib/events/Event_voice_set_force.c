

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
#include <Event_voice_set_force.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_force_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = 18
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_set_force,
                                   EVENT_VOICE_SET_FORCE,
                                   double, force_dB)


static void Event_voice_set_force_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_set_force,
                         EVENT_VOICE_SET_FORCE,
                         set_force_desc,
                         event->force_dB = 0)


static void Event_voice_set_force_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FORCE);
    assert(voice != NULL);
    Event_voice_set_force* set_force = (Event_voice_set_force*)event;
    voice->state.generic.force = exp2(set_force->force_dB / 6);
    voice->state.generic.force_slide = 0;
    return;
}


