

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
#include <Event_voice_slide_force.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_force_desc[] =
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


Event_create_set_primitive_and_get(Event_voice_slide_force,
                                   EVENT_VOICE_SLIDE_FORCE,
                                   double, target_force_dB)


static void Event_voice_slide_force_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_force,
                         EVENT_VOICE_SLIDE_FORCE,
                         slide_force_desc,
                         event->target_force_dB = 0)


static void Event_voice_slide_force_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_FORCE);
    assert(voice != NULL);
    assert(voice->state.generic.tempo > 0);
    assert(voice->state.generic.freq > 0);
    Event_voice_slide_force* slide_force = (Event_voice_slide_force*)event;
    voice->state.generic.force_slide_target = exp2(slide_force->target_force_dB / 6);
    voice->state.generic.force_slide_frames =
            Reltime_toframes(&voice->state.generic.force_slide_length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    double force_dB = log2(voice->state.generic.force) * 6;
    double dB_step = (slide_force->target_force_dB - force_dB) /
            voice->state.generic.force_slide_frames;
    voice->state.generic.force_slide_update = exp2(dB_step / 6);
    if (dB_step > 0)
    {
        voice->state.generic.force_slide = 1;
    }
    else if (dB_step < 0)
    {
        voice->state.generic.force_slide = -1;
    }
    else
    {
        voice->state.generic.force = voice->state.generic.force_slide_target;
    }
    return;
}


