

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
#include <Event_voice_set_panning.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_panning_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -1, 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_set_panning,
                                   EVENT_VOICE_SET_PANNING,
                                   double, panning)


static void Event_voice_set_panning_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_set_panning,
                         EVENT_VOICE_SET_PANNING,
                         set_panning_desc,
                         event->panning = 0)


static void Event_voice_set_panning_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_PANNING);
    assert(voice != NULL);
    Event_voice_set_panning* set_panning = (Event_voice_set_panning*)event;
    voice->state.generic.panning = set_panning->panning;
    voice->state.generic.panning_slide = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->panning = set_panning->panning;
    ch_state->panning_slide = 0;
    return;
}


