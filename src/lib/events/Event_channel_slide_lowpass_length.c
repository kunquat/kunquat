

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <Event_channel_slide_lowpass_length.h>
#include <Tstamp.h>
#include <Voice.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_slide_lowpass_length_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    Tstamp_copy(
            &ch_state->filter_slide_length,
            &value->value.Tstamp_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        Slider_set_length(&vs->lowpass_slider, &value->value.Tstamp_type);
    }
    return true;
}


