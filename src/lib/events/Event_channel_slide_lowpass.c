

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
#include <Event_channel_slide_lowpass.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>


bool Event_channel_slide_lowpass_process(Channel_state* ch_state,
                                         Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double target_cutoff = value->value.float_type;
    double target_cutoff_exp = exp2((target_cutoff + 86) / 12);
    const double inf_limit = exp2((86.0 + 86) / 12);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        if (Slider_in_progress(&vs->lowpass_slider))
        {
            Slider_change_target(&vs->lowpass_slider, target_cutoff_exp);
        }
        else
        {
            Slider_start(&vs->lowpass_slider,
                         target_cutoff_exp,
                         isfinite(vs->lowpass) ? vs->lowpass : inf_limit);
        }
    }
    return true;
}


