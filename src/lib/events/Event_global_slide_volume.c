

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
#include <limits.h>
#include <math.h>

#include <Event_common.h>
#include <Event_global_slide_volume.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_slide_volume_process(Playdata* global_state, Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double target = exp2(value->value.float_type / 6);
    Slider_set_mix_rate(&global_state->volume_slider, global_state->freq);
    Slider_set_tempo(&global_state->volume_slider, global_state->tempo);
    if (Slider_in_progress(&global_state->volume_slider))
    {
        Slider_change_target(&global_state->volume_slider, target);
    }
    else
    {
        Slider_start(
                &global_state->volume_slider,
                target,
                global_state->volume);
    }
    return true;
}


