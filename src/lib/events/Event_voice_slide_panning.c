

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
#include <limits.h>

#include <Event_common.h>
#include <Event_voice_slide_panning.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_panning_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { -1, 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_slide_panning,
                                   EVENT_VOICE_SLIDE_PANNING,
                                   double, target_panning)


static void Event_voice_slide_panning_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_panning,
                         EVENT_VOICE_SLIDE_PANNING,
                         slide_panning_desc,
                         event->target_panning = 0)


static void Event_voice_slide_panning_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_PANNING);
    assert(voice != NULL);
    Event_voice_slide_panning* slide_panning = (Event_voice_slide_panning*)event;
    voice->state.generic.panning_slide_target = slide_panning->target_panning;
    voice->state.generic.panning_slide_frames =
            Reltime_toframes(&voice->state.generic.panning_slide_length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    double diff = voice->state.generic.panning_slide_target -
            voice->state.generic.panning;
    voice->state.generic.panning_slide_update = diff /
            voice->state.generic.panning_slide_frames;
    if (diff > 0)
    {
        voice->state.generic.panning_slide = 1;
        ch_state->panning_slide = 1;
    }
    else if (diff < 0)
    {
        voice->state.generic.panning_slide = -1;
        ch_state->panning_slide = -1;
    }
    else
    {
        voice->state.generic.panning = voice->state.generic.panning_slide_target;
        voice->state.generic.panning_slide = 0;
        ch_state->panning = slide_panning->target_panning;
        ch_state->panning_slide = 0;
    }
    return;
}


