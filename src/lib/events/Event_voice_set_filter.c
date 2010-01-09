

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_set_filter.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_filter_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -INFINITY, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_set_filter,
                                   EVENT_VOICE_SET_FILTER,
                                   double, cutoff)


static void Event_voice_set_filter_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_set_filter,
                         EVENT_VOICE_SET_FILTER,
                         set_filter_desc,
                         event->cutoff = INFINITY)


static void Event_voice_set_filter_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FILTER);
    assert(voice != NULL);
    Event_voice_set_filter* set_filter = (Event_voice_set_filter*)event;
    if (set_filter->cutoff > 86)
    {
        voice->state.generic.filter = INFINITY;
        return;
    }
    voice->state.generic.filter = exp2((set_filter->cutoff + 86) / 12);
    return;
}


