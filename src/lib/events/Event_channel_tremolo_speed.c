

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

#include <Event_common.h>
#include <Event_channel_tremolo_speed.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc tremolo_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = 0,
        .max.field.double_type = INFINITY
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_channel_tremolo_speed,
                                   EVENT_CHANNEL_TREMOLO_SPEED,
                                   double, speed)


static void Event_channel_tremolo_speed_process(Event_channel* event, Channel* ch);


Event_create_constructor(Event_channel_tremolo_speed,
                         EVENT_CHANNEL_TREMOLO_SPEED,
                         tremolo_speed_desc,
                         event->speed = 0)


bool Event_channel_tremolo_speed_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, tremolo_speed_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       *ch_state->tempo,
                                       *ch_state->freq);
    ch_state->tremolo_length = unit_len / data[0].field.double_type;
    ch_state->tremolo_update = (2 * PI) / ch_state->tremolo_length;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        if (data[0].field.double_type > 0 && vs->tremolo_depth_target > 0)
        {
            vs->tremolo = true;
        }
        vs->tremolo_length = ch_state->tremolo_length;
        vs->tremolo_update = ch_state->tremolo_update;
        if (!vs->tremolo)
        {
            vs->tremolo_delay_pos = 0;
        }
    }
    return true;
}


static void Event_channel_tremolo_speed_process(Event_channel* event, Channel* ch)
{
    (void)event;
    (void)ch;
    assert(false);
#if 0
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO_SPEED);
    assert(voice != NULL);
    Event_channel_tremolo_speed* tremolo_speed = (Event_channel_tremolo_speed*)event;
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
#endif
}


