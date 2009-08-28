

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_voice_slide_panning_length.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_panning_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_voice_slide_panning_length,
                                 EVENT_VOICE_SLIDE_PANNING_LENGTH,
                                 length)


static void Event_voice_slide_panning_length_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_panning_length,
                         EVENT_VOICE_SLIDE_PANNING_LENGTH,
                         slide_panning_length_desc,
                         Reltime_set(&event->length, 0, 0))


static void Event_voice_slide_panning_length_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_PANNING_LENGTH);
    assert(voice != NULL);
    Event_voice_slide_panning_length* slide_panning_length = (Event_voice_slide_panning_length*)event;
    voice->state.generic.panning_slide_frames =
            Reltime_toframes(&slide_panning_length->length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    Reltime_copy(&voice->state.generic.panning_slide_length, &slide_panning_length->length);
    if (voice->state.generic.panning_slide != 0)
    {
        double diff = voice->state.generic.panning_slide_target -
                      voice->state.generic.panning;
        voice->state.generic.panning_slide_update = diff /
                voice->state.generic.panning_slide_frames;
    }
    Channel_state* ch_state = voice->state.generic.cur_ch_state;
    Reltime_copy(&ch_state->panning_slide_length, &slide_panning_length->length);
    ch_state = voice->state.generic.new_ch_state;
    Reltime_copy(&ch_state->panning_slide_length, &slide_panning_length->length);
    return;
}


