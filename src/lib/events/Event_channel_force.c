

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

#include <Event_channel_decl.h>
#include <Event_common.h>
#include <player/Voice.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_set_force_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    double force = exp2(value->value.float_type / 6);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        // Update actual force in case it's queried before another render call
        vs->actual_force *= (force / vs->force);

        vs->force = force;
        Slider_break(&vs->force_slider);
//        vs->force_slide = 0;
    }

    return true;
}


bool Event_channel_slide_force_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    double slide_target = exp2(value->value.float_type / 6);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (Slider_in_progress(&vs->force_slider))
            Slider_change_target(&vs->force_slider, slide_target);
        else
            Slider_start(&vs->force_slider, slide_target, vs->force);
    }

    return true;
}


bool Event_channel_slide_force_length_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->force_slide_length, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_set_length(&vs->force_slider, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_tremolo_speed_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    ch->tremolo_speed = value->value.float_type;
    LFO_set_speed(&ch->tremolo, value->value.float_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_speed(&vs->tremolo, value->value.float_type);

        if (ch->tremolo_depth > 0)
            LFO_set_depth(&vs->tremolo, ch->tremolo_depth);

        LFO_turn_on(&vs->tremolo);
    }

    return true;
}


bool Event_channel_tremolo_depth_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    double actual_depth = value->value.float_type / 6;
    ch->tremolo_depth = actual_depth;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (ch->tremolo_speed > 0)
            LFO_set_speed(&vs->tremolo, ch->tremolo_speed);

        LFO_set_depth(&vs->tremolo, actual_depth);
        LFO_turn_on(&vs->tremolo);
    }

    return true;
}


bool Event_channel_tremolo_delay_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(
            &ch->tremolo_depth_delay,
            &value->value.Tstamp_type);
    LFO_set_depth_delay(&ch->tremolo, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_depth_delay(&vs->tremolo, &value->value.Tstamp_type);
    }

    return true;
}


