

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
#include <Event_channel_set_filter.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc set_filter_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = INFINITY
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_channel_set_filter,
                                   EVENT_CHANNEL_SET_FILTER,
                                   double, cutoff)


static void Event_channel_set_filter_process(Event_channel* event, Channel* ch);


Event_create_constructor(Event_channel_set_filter,
                         EVENT_CHANNEL_SET_FILTER,
                         set_filter_desc,
                         event->cutoff = INFINITY)


bool Event_channel_set_filter_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_filter_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double cutoff = NAN;
    if (data[0].field.double_type > 86)
    {
        cutoff = INFINITY;
    }
    else
    {
        cutoff = exp2((data[0].field.double_type + 86) / 12);
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        vs->filter = cutoff;
    }
    return true;
}


static void Event_channel_set_filter_process(Event_channel* event, Channel* ch)
{
    (void)event;
    (void)ch;
    assert(false);
#if 0
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SET_FILTER);
    assert(voice != NULL);
    Event_channel_set_filter* set_filter = (Event_channel_set_filter*)event;
    if (set_filter->cutoff > 86)
    {
        voice->state.generic.filter = INFINITY;
        return;
    }
    voice->state.generic.filter = exp2((set_filter->cutoff + 86) / 12);
    return;
#endif
}


