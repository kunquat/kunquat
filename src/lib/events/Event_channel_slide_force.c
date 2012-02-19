

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
#include <Event_channel_slide_force.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SLIDE_FORCE,
                         slide_force);


bool Event_channel_slide_force_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double slide_target = exp2(value->value.float_type / 6);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        if (Slider_in_progress(&vs->force_slider))
        {
            Slider_change_target(&vs->force_slider, slide_target);
        }
        else
        {
            Slider_start(&vs->force_slider, slide_target, vs->force);
        }
    }
    return true;
}


