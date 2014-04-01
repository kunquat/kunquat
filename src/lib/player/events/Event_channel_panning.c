

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <player/events/Event_channel_decl.h>
#include <player/events/Event_common.h>
#include <player/Voice.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_set_panning_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    ch->panning = value->value.float_type;
    Slider_break(&ch->panning_slider);
//    ch->panning_slide = 0;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        vs->panning = ch->panning;
        Slider_break(&vs->panning_slider);
//        vs->panning_slide = 0;
    }

    return true;
}


bool Event_channel_slide_panning_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    if (Slider_in_progress(&ch->panning_slider))
        Slider_change_target(
                &ch->panning_slider,
                value->value.float_type);
    else
        Slider_start(
                &ch->panning_slider,
                value->value.float_type,
                ch->panning);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_copy(&vs->panning_slider, &ch->panning_slider);
    }

    return true;
}


bool Event_channel_slide_panning_length_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Slider_set_length(
            &ch->panning_slider,
            &value->value.Tstamp_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_set_length(&vs->panning_slider, &value->value.Tstamp_type);
    }

    return true;
}


