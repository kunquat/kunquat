

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
#include <Value.h>
#include <Voice.h>
#include <xassert.h>


bool Event_channel_set_lowpass_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double cutoff = NAN;
    if (value->value.float_type > 86)
    {
        cutoff = INFINITY;
    }
    else
    {
        cutoff = exp2((value->value.float_type + 86) / 12);
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->lowpass = cutoff;
    }
    return true;
}


bool Event_channel_slide_lowpass_process(
        Channel_state* ch_state,
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


bool Event_channel_autowah_speed_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ch_state->autowah_speed = value->value.float_type;
    LFO_set_speed(&ch_state->autowah, value->value.float_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        LFO_set_speed(&vs->autowah, value->value.float_type);
        if (ch_state->autowah_depth > 0)
        {
            LFO_set_depth(&vs->autowah, ch_state->autowah_depth);
        }
        LFO_turn_on(&vs->autowah);
    }
    return true;
}


bool Event_channel_autowah_depth_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double actual_depth = value->value.float_type / 8;
    ch_state->autowah_depth = actual_depth;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        if (ch_state->autowah_speed > 0)
        {
            LFO_set_speed(&vs->autowah, ch_state->autowah_speed);
        }
        LFO_set_depth(&vs->autowah, actual_depth);
        LFO_turn_on(&vs->autowah);
    }
    return true;
}


bool Event_channel_autowah_delay_process(
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
            &ch_state->autowah_depth_delay,
            &value->value.Tstamp_type);
    LFO_set_depth_delay(&ch_state->autowah, &value->value.Tstamp_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        LFO_set_depth_delay(&vs->autowah, &value->value.Tstamp_type);
    }
    return true;
}


bool Event_channel_set_resonance_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double resonance = pow(1.055, value->value.float_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->lowpass_resonance = resonance;
    }
    return true;
}


