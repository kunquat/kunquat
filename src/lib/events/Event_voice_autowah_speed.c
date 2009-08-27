

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

#include <Event_common.h>
#include <Event_voice_autowah_speed.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc autowah_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


create_set_primitive_and_get(Event_voice_autowah_speed,
                             EVENT_VOICE_AUTOWAH_SPEED,
                             double, speed)


static void Event_voice_autowah_speed_process(Event_voice* event, Voice* voice);


create_constructor(Event_voice_autowah_speed,
                   EVENT_VOICE_AUTOWAH_SPEED,
                   autowah_speed_desc,
                   event->speed = 0)


static void Event_voice_autowah_speed_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_AUTOWAH_SPEED);
    assert(voice != NULL);
    Event_voice_autowah_speed* autowah_speed = (Event_voice_autowah_speed*)event;
    if (autowah_speed->speed > 0 && voice->state.generic.autowah_depth_target > 0)
    {
        voice->state.generic.autowah = true;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       voice->state.generic.tempo,
                                       voice->state.generic.freq);
    voice->state.generic.autowah_length = unit_len / autowah_speed->speed;
    voice->state.generic.autowah_update = (2 * PI) / voice->state.generic.autowah_length;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->autowah_length = voice->state.generic.autowah_length;
    ch_state->autowah_update = voice->state.generic.autowah_update;
    if (!voice->state.generic.autowah)
    {
        voice->state.generic.autowah_delay_pos = 0;
    }
    return;
}


