

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
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


Event_create_set_primitive_and_get(Event_voice_vibrato_speed,
                                   EVENT_VOICE_VIBRATO_SPEED,
                                   double, speed)


static void Event_voice_vibrato_speed_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_vibrato_speed,
                         EVENT_VOICE_VIBRATO_SPEED,
                         vibrato_speed_desc,
                         event->speed = 0)


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


