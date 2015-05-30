

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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

#include <debug/assert.h>
#include <mathnum/common.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_common.h>
#include <player/Voice.h>
#include <Value.h>


bool Event_channel_set_lowpass_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        vs->lowpass = value->value.float_type;

        Slider_break(&vs->lowpass_slider);
    }

    return true;
}


bool Event_channel_slide_lowpass_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const double target_lowpass = value->value.float_type;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        if (Slider_in_progress(&vs->lowpass_slider))
            Slider_change_target(&vs->lowpass_slider, target_lowpass);
        else
            Slider_start(&vs->lowpass_slider, target_lowpass, vs->lowpass);
    }

    return true;
}


bool Event_channel_slide_lowpass_length_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->filter_slide_length, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_set_length(&vs->lowpass_slider, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_autowah_speed_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    ch->autowah_speed = value->value.float_type;
    LFO_set_speed(&ch->autowah, value->value.float_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_speed(&vs->autowah, value->value.float_type);

        if (ch->autowah_depth > 0)
            LFO_set_depth(&vs->autowah, ch->autowah_depth);

        LFO_turn_on(&vs->autowah);
    }

    return true;
}


bool Event_channel_autowah_depth_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const double actual_depth = value->value.float_type;
    ch->autowah_depth = actual_depth;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (ch->autowah_speed > 0)
            LFO_set_speed(&vs->autowah, ch->autowah_speed);

        LFO_set_depth(&vs->autowah, actual_depth);
        LFO_turn_on(&vs->autowah);
    }

    return true;
}


bool Event_channel_autowah_speed_slide_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->autowah_speed_slide, &value->value.Tstamp_type);
    LFO_set_speed_slide(&ch->autowah, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_speed_slide(&vs->autowah, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_autowah_depth_slide_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->autowah_depth_slide, &value->value.Tstamp_type);
    LFO_set_depth_slide(&ch->autowah, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_depth_slide(&vs->autowah, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_set_resonance_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const double resonance = value->value.float_type;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        vs->lowpass_resonance = resonance;
    }

    return true;
}


