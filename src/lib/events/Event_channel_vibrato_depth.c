

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
#include <Event_channel_vibrato_depth.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc vibrato_depth_desc[] =
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


Event_create_set_primitive_and_get(Event_channel_vibrato_depth,
                                   EVENT_CHANNEL_VIBRATO_DEPTH,
                                   double, depth)


static void Event_channel_vibrato_depth_process(Event_channel* event, Channel* ch);


Event_create_constructor(Event_channel_vibrato_depth,
                         EVENT_CHANNEL_VIBRATO_DEPTH,
                         vibrato_depth_desc,
                         event->depth = 0)


bool Event_channel_vibrato_depth_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, vibrato_depth_desc, data, state);
    if (state->error)
    {
        return false;
    }
    ch_state->vibrato_depth = data[0].field.double_type / 240; // unit is 5 cents
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        if (data[0].field.double_type > 0 && vs->vibrato_length > 0)
        {
            vs->vibrato = true;
        }
        vs->vibrato_depth_target = ch_state->vibrato_depth;
        vs->vibrato_delay_pos = 0;
    }
    return true;
}


static void Event_channel_vibrato_depth_process(Event_channel* event, Channel* ch)
{
    (void)event;
    (void)ch;
    assert(false);
#if 0
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_VIBRATO_DEPTH);
    assert(voice != NULL);
    Event_channel_vibrato_depth* vibrato_depth = (Event_channel_vibrato_depth*)event;
    if (vibrato_depth->depth > 0 && voice->state.generic.vibrato_length > 0)
    {
        voice->state.generic.vibrato = true;
    }
    voice->state.generic.vibrato_depth_target = vibrato_depth->depth / 240; // unit is 5 cents
    voice->state.generic.vibrato_delay_pos = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->vibrato_depth = voice->state.generic.vibrato_depth_target;
    return;
#endif
}


