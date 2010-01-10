

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
#include <Event_voice_slide_filter_length.h>
#include <Reltime.h>
#include <Voice.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_filter_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_voice_slide_filter_length,
                                 EVENT_VOICE_SLIDE_FILTER_LENGTH,
                                 length)


static void Event_voice_slide_filter_length_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_filter_length,
                         EVENT_VOICE_SLIDE_FILTER_LENGTH,
                         slide_filter_length_desc,
                         Reltime_set(&event->length, 0, 0))


static void Event_voice_slide_filter_length_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_FILTER_LENGTH);
    assert(voice != NULL);
    Event_voice_slide_filter_length* slide_filter_length = (Event_voice_slide_filter_length*)event;
    voice->state.generic.filter_slide_frames =
            Reltime_toframes(&slide_filter_length->length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    Reltime_copy(&voice->state.generic.filter_slide_length, &slide_filter_length->length);
    if (voice->state.generic.filter_slide != 0)
    {
        double diff_log = (log2(voice->state.generic.filter_slide_target) * 12 - 86) -
                          (log2(voice->state.generic.filter) * 12 - 86);
        double slide_step = diff_log / voice->state.generic.filter_slide_frames;
        voice->state.generic.filter_slide_update = exp2(slide_step / 12);
    }
    Channel_state* ch_state = voice->state.generic.cur_ch_state;
    Reltime_copy(&ch_state->filter_slide_length, &slide_filter_length->length);
    ch_state = voice->state.generic.new_ch_state;
    Reltime_copy(&ch_state->filter_slide_length, &slide_filter_length->length);
    return;
}


