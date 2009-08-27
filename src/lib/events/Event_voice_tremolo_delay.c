

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
#include <stdint.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_tremolo_delay.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc tremolo_delay_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_reltime_and_get(Event_voice_tremolo_delay,
                           EVENT_VOICE_TREMOLO_DELAY,
                           delay)


static void Event_voice_tremolo_delay_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_tremolo_delay(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_tremolo_delay* event = xalloc(Event_voice_tremolo_delay);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_TREMOLO_DELAY,
               tremolo_delay_desc,
               Event_voice_tremolo_delay_set,
               Event_voice_tremolo_delay_get);
    event->parent.process = Event_voice_tremolo_delay_process;
    Reltime_set(&event->delay, 0, KQT_RELTIME_BEAT / 4);
    return (Event*)event;
}


static void Event_voice_tremolo_delay_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO_DELAY);
    assert(voice != NULL);
    Event_voice_tremolo_delay* tremolo_delay = (Event_voice_tremolo_delay*)event;
    double delay_frames = Reltime_toframes(&tremolo_delay->delay,
                                           voice->state.generic.tempo,
                                           voice->state.generic.freq);
    voice->state.generic.tremolo_delay_pos = 0;
    voice->state.generic.tremolo_delay_update = 1 / delay_frames;
    if (voice->state.generic.tremolo_delay_update == 0)
    {
        voice->state.generic.tremolo_delay_pos = 1;
    }
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->tremolo_delay_update = voice->state.generic.tremolo_delay_update;
    return;
}


