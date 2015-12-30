

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


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/Voice.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


bool Event_channel_set_lowpass_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    ch->filter_controls.lowpass = value->value.float_type;
    Slider_break(&ch->filter_controls.lowpass_slider);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        vs->filter_controls.lowpass = ch->filter_controls.lowpass;
        Slider_break(&vs->filter_controls.lowpass_slider);
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

    if (Slider_in_progress(&ch->filter_controls.lowpass_slider))
        Slider_change_target(&ch->filter_controls.lowpass_slider, target_lowpass);
    else
        Slider_start(
                &ch->filter_controls.lowpass_slider,
                target_lowpass,
                ch->filter_controls.lowpass);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (Slider_in_progress(&vs->filter_controls.lowpass_slider))
            Slider_change_target(&vs->filter_controls.lowpass_slider, target_lowpass);
        else
            Slider_start(
                    &vs->filter_controls.lowpass_slider,
                    target_lowpass,
                    vs->filter_controls.lowpass);
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

    Slider_set_length(&ch->filter_controls.lowpass_slider, &ch->filter_slide_length);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_set_length(&vs->filter_controls.lowpass_slider, &ch->filter_slide_length);
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

    LFO_set_speed(&ch->filter_controls.autowah, ch->autowah_speed);

    if (ch->autowah_depth > 0)
        LFO_set_depth(&ch->filter_controls.autowah, ch->autowah_depth);

    LFO_turn_on(&ch->filter_controls.autowah);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        LFO_set_speed(&vs->filter_controls.autowah, ch->autowah_speed);

        if (ch->autowah_depth > 0)
            LFO_set_depth(&vs->filter_controls.autowah, ch->autowah_depth);

        LFO_turn_on(&vs->filter_controls.autowah);
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

    ch->autowah_depth = value->value.float_type;

    if (ch->autowah_speed > 0)
        LFO_set_speed(&ch->filter_controls.autowah, ch->autowah_speed);

    LFO_set_depth(&ch->filter_controls.autowah, ch->autowah_depth);

    LFO_turn_on(&ch->filter_controls.autowah);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (ch->autowah_speed > 0)
            LFO_set_speed(&vs->filter_controls.autowah, ch->autowah_speed);

        LFO_set_depth(&vs->filter_controls.autowah, ch->autowah_depth);
        LFO_turn_on(&vs->filter_controls.autowah);
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

    LFO_set_speed_slide(&ch->filter_controls.autowah, &ch->autowah_speed_slide);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_speed_slide(&vs->filter_controls.autowah, &ch->autowah_speed_slide);
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

    LFO_set_depth_slide(&ch->filter_controls.autowah, &ch->autowah_depth_slide);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_depth_slide(&vs->filter_controls.autowah, &ch->autowah_depth_slide);
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

    ch->filter_controls.resonance = resonance;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        vs->filter_controls.resonance = resonance;
    }

    return true;
}


bool Event_channel_slide_resonance_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const double target_resonance = value->value.float_type;

    if (Slider_in_progress(&ch->filter_controls.resonance_slider))
        Slider_change_target(&ch->filter_controls.resonance_slider, target_resonance);
    else
        Slider_start(
                &ch->filter_controls.resonance_slider,
                target_resonance,
                ch->filter_controls.resonance);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (Slider_in_progress(&vs->filter_controls.resonance_slider))
            Slider_change_target(
                    &vs->filter_controls.resonance_slider, target_resonance);
        else
            Slider_start(
                    &vs->filter_controls.resonance_slider,
                    target_resonance,
                    vs->filter_controls.resonance);
    }

    return true;
}


bool Event_channel_slide_resonance_length_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->lowpass_resonance_slide_length, &value->value.Tstamp_type);

    Slider_set_length(
            &ch->filter_controls.resonance_slider, &ch->lowpass_resonance_slide_length);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Slider_set_length(
                &vs->filter_controls.resonance_slider,
                &ch->lowpass_resonance_slide_length);
    }

    return true;
}


bool Event_channel_carry_filter_on_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    ch->carry_filter = true;

    return true;
}


bool Event_channel_carry_filter_off_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    ch->carry_filter = false;

    return true;
}


