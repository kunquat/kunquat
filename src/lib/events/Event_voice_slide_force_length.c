

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
#include <Event_voice_slide_force_length.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_force_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .min.field.Reltime_type = { 0, 0 },
        .max.field.Reltime_type = { INT64_MAX, KQT_RELTIME_BEAT - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_voice_slide_force_length,
                                 EVENT_VOICE_SLIDE_FORCE_LENGTH,
                                 length)


static void Event_voice_slide_force_length_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_force_length,
                         EVENT_VOICE_SLIDE_FORCE_LENGTH,
                         slide_force_length_desc,
                         Reltime_set(&event->length, 0, 0))


static void Event_voice_slide_force_length_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_FORCE_LENGTH);
    assert(voice != NULL);
    Event_voice_slide_force_length* slide_force_length = (Event_voice_slide_force_length*)event;
    voice->state.generic.force_slide_frames =
            Reltime_toframes(&slide_force_length->length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    Reltime_copy(&voice->state.generic.force_slide_length, &slide_force_length->length);
    if (voice->state.generic.force_slide != 0)
    {
        double force_dB = log2(voice->state.generic.force) * 6;
        double target_dB = log2(voice->state.generic.force_slide_target) * 6;
        double dB_step = (target_dB - force_dB) / voice->state.generic.force_slide_frames;
        voice->state.generic.force_slide_update = exp2(dB_step / 6);
    }
    Channel_state* ch_state = voice->state.generic.cur_ch_state;
    Reltime_copy(&ch_state->force_slide_length, &slide_force_length->length);
    ch_state = voice->state.generic.new_ch_state;
    Reltime_copy(&ch_state->force_slide_length, &slide_force_length->length);
    return;
}


