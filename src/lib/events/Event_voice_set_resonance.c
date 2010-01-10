

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_set_resonance.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_resonance_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, 99 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_set_resonance,
                                   EVENT_VOICE_SET_RESONANCE,
                                   double, resonance)


static void Event_voice_set_resonance_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_set_resonance,
                         EVENT_VOICE_SET_RESONANCE,
                         set_resonance_desc,
                         event->resonance = 1)


static void Event_voice_set_resonance_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_RESONANCE);
    assert(voice != NULL);
    Event_voice_set_resonance* set_resonance = (Event_voice_set_resonance*)event;
    voice->state.generic.filter_resonance = pow(1.055, set_resonance->resonance);
    return;
}


