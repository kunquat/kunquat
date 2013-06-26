

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
#include <float.h>

#include <Event_channel_decl.h>
#include <Event_common.h>
#include <kunquat/limits.h>
#include <Scale.h>
#include <Voice.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_slide_pitch_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice* voice = ch_state->fg[i];
        if (voice->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
        Voice_state* vs = voice->state;
        pitch_t pitch = -1;
        if (voice->gen->ins_params->scale == NULL ||
                *voice->gen->ins_params->scale == NULL ||
                **voice->gen->ins_params->scale == NULL)
        {
            pitch = value->value.float_type;
        }
        else
        {
            pitch = Scale_get_pitch_from_cents(
                    **voice->gen->ins_params->scale,
                    value->value.float_type);
        }
        if (pitch <= 0)
        {
            continue;
        }
        if (Slider_in_progress(&vs->pitch_slider))
        {
            Slider_change_target(&vs->pitch_slider, pitch);
        }
        else
        {
            Slider_start(&vs->pitch_slider, pitch, vs->pitch);
        }
    }
    return true;
}


bool Event_channel_slide_pitch_length_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    Tstamp_copy(&ch_state->pitch_slide_length, &value->value.Tstamp_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        Slider_set_length(&vs->pitch_slider, &value->value.Tstamp_type);
    }
    return true;
}


bool Event_channel_vibrato_speed_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ch_state->vibrato_speed = value->value.float_type;
    LFO_set_speed(&ch_state->vibrato, value->value.float_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        if (ch_state->fg[i]->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
        Voice_state* vs = ch_state->fg[i]->state;
        LFO_set_speed(&vs->vibrato, value->value.float_type);
        if (ch_state->vibrato_depth > 0)
        {
            LFO_set_depth(&vs->vibrato, ch_state->vibrato_depth);
        }
        LFO_turn_on(&vs->vibrato);
    }
    return true;
}


bool Event_channel_vibrato_depth_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    double actual_depth = value->value.float_type / 240; // unit is 5 cents
    ch_state->vibrato_depth = actual_depth;
    LFO_set_depth(&ch_state->vibrato, actual_depth); // unit is 5 cents
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        if (ch_state->fg[i]->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
        Voice_state* vs = ch_state->fg[i]->state;
        if (ch_state->vibrato_speed > 0)
        {
            LFO_set_speed(&vs->vibrato, ch_state->vibrato_speed);
        }
        LFO_set_depth(&vs->vibrato, actual_depth);
        LFO_turn_on(&vs->vibrato);
    }
    return true;
}


bool Event_channel_vibrato_delay_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    Tstamp_copy(&ch_state->vibrato_depth_delay,
                 &value->value.Tstamp_type);
    LFO_set_depth_delay(&ch_state->vibrato, &value->value.Tstamp_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        LFO_set_depth_delay(&vs->vibrato, &value->value.Tstamp_type);
    }
    return true;
}


