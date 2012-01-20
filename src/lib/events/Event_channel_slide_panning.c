

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
#include <limits.h>

#include <Event_common.h>
#include <Event_channel_slide_panning.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc slide_panning_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -1,
        .max.field.double_type = 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SLIDE_PANNING,
                         slide_panning);


bool Event_channel_slide_panning_process(Channel_state* ch_state,
                                         Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    if (Slider_in_progress(&ch_state->panning_slider))
    {
        Slider_change_target(&ch_state->panning_slider,
                             value->value.float_type);
    }
    else
    {
        Slider_start(&ch_state->panning_slider,
                     value->value.float_type,
                     ch_state->panning);
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        Slider_copy(&vs->panning_slider, &ch_state->panning_slider);
    }
    return true;
}


