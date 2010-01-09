

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_tremolo_speed.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc tremolo_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_tremolo_speed,
                                   EVENT_VOICE_TREMOLO_SPEED,
                                   double, speed)


static void Event_voice_tremolo_speed_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_tremolo_speed,
                         EVENT_VOICE_TREMOLO_SPEED,
                         tremolo_speed_desc,
                         event->speed = 0)


static void Event_voice_tremolo_speed_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO_SPEED);
    assert(voice != NULL);
    Event_voice_tremolo_speed* tremolo_speed = (Event_voice_tremolo_speed*)event;
    if (tremolo_speed->speed > 0 && voice->state.generic.tremolo_depth_target > 0)
    {
        voice->state.generic.tremolo = true;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       voice->state.generic.tempo,
                                       voice->state.generic.freq);
    voice->state.generic.tremolo_length = unit_len / tremolo_speed->speed;
    voice->state.generic.tremolo_update = (2 * PI) / voice->state.generic.tremolo_length;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->tremolo_length = voice->state.generic.tremolo_length;
    ch_state->tremolo_update = voice->state.generic.tremolo_update;
    if (!voice->state.generic.tremolo)
    {
        voice->state.generic.tremolo_delay_pos = 0;
    }
    return;
}


