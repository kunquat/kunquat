

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_channel_set_panning.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SET_PANNING,
                         set_panning);


bool Event_channel_set_panning_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ch_state->panning = value->value.float_type;
    Slider_break(&ch_state->panning_slider);
//    ch_state->panning_slide = 0;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->panning = ch_state->panning;
        Slider_break(&vs->panning_slider);
//        vs->panning_slide = 0;
    }
    return true;
}


