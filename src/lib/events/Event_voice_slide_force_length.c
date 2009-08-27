

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
#include <Event_voice_slide_force_length.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_force_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_reltime_and_get(Event_voice_slide_force_length,
                           EVENT_VOICE_SLIDE_FORCE_LENGTH,
                           length)


static void Event_voice_slide_force_length_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_slide_force_length(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_slide_force_length* event = xalloc(Event_voice_slide_force_length);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_SLIDE_FORCE_LENGTH,
               slide_force_length_desc,
               Event_voice_slide_force_length_set,
               Event_voice_slide_force_length_get);
    event->parent.process = Event_voice_slide_force_length_process;
    Reltime_set(&event->length, 0, 0);
    return (Event*)event;
}


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


