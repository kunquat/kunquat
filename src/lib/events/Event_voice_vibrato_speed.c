

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
#include <Event_voice_vibrato_speed.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc vibrato_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_vibrato_speed_set(Event* event, int index, void* data);

static void* Event_voice_vibrato_speed_get(Event* event, int index);

static void Event_voice_vibrato_speed_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_vibrato_speed(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_vibrato_speed* event = xalloc(Event_voice_vibrato_speed);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_VIBRATO_SPEED,
               vibrato_speed_desc,
               Event_voice_vibrato_speed_set,
               Event_voice_vibrato_speed_get);
    event->parent.process = Event_voice_vibrato_speed_process;
    event->speed = 0;
    return (Event*)event;
}


static bool Event_voice_vibrato_speed_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_VIBRATO_SPEED);
    assert(data != NULL);
    Event_voice_vibrato_speed* vibrato_speed = (Event_voice_vibrato_speed*)event;
    if (index != 0)
    {
        return false;
    }
    double speed = *(double*)data;
    Event_check_double_range(speed, event->field_types[0]);
    vibrato_speed->speed = speed;
    return true;
}


static void* Event_voice_vibrato_speed_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_VIBRATO_SPEED);
    Event_voice_vibrato_speed* vibrato_speed = (Event_voice_vibrato_speed*)event;
    if (index != 0)
    {
        return NULL;
    }
    return &vibrato_speed->speed;
}


static void Event_voice_vibrato_speed_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_VIBRATO_SPEED);
    assert(voice != NULL);
    Event_voice_vibrato_speed* vibrato_speed = (Event_voice_vibrato_speed*)event;
    if (vibrato_speed->speed > 0 && voice->state.generic.vibrato_depth_target > 0)
    {
        voice->state.generic.vibrato = true;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       voice->state.generic.tempo,
                                       voice->state.generic.freq);
    voice->state.generic.vibrato_length = unit_len / vibrato_speed->speed;
    voice->state.generic.vibrato_update = (2 * PI) / voice->state.generic.vibrato_length;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->vibrato_length = voice->state.generic.vibrato_length;
    ch_state->vibrato_update = voice->state.generic.vibrato_update;
    if (!voice->state.generic.vibrato)
    {
        voice->state.generic.vibrato_delay_pos = 0;
    }
    return;
}


