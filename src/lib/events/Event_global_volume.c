

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

#include <Event_common.h>
#include <Event_master_decl.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_set_volume_process(
        Master_params* master_params,
        Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    return false;

#if 0
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    global_state->volume = exp2(value->value.float_type / 6);
    Slider_break(&global_state->volume_slider);
//    global_state->volume_slide = 0;
    return true;
#endif
}


bool Event_global_slide_volume_process(
        Master_params* master_params,
        Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    return false;

#if 0
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
#endif
}


bool Event_global_slide_volume_length_process(
        Master_params* master_params,
        Value* value)
{
    assert(master_params != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    return false;

#if 0
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    Slider_set_mix_rate(&global_state->volume_slider, global_state->freq);
    Slider_set_tempo(&global_state->volume_slider, global_state->tempo);
    Slider_set_length(&global_state->volume_slider,
                      &value->value.Tstamp_type);
    return true;
#endif
}


