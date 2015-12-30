

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
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/Voice.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


bool Event_channel_set_force_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const double force = exp2(value->value.float_type / 6);

    ch->force_controls.force = force;
    Slider_break(&ch->force_controls.slider);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        // Update actual force in case it's queried before another render call
        vs->actual_force *= (force / vs->force_controls.force);

        vs->force_controls.force = force;
        Slider_break(&vs->force_controls.slider);
//        vs->force_slide = 0;
    }

    return true;
}


bool Event_channel_slide_force_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    double slide_target = exp2(value->value.float_type / 6);

    if (Slider_in_progress(&ch->force_controls.slider))
        Slider_change_target(&ch->force_controls.slider, slide_target);
    else
        Slider_start(&ch->force_controls.slider, slide_target, ch->force_controls.force);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (Slider_in_progress(&vs->force_controls.slider))
            Slider_change_target(&vs->force_controls.slider, slide_target);
        else
            Slider_start(
                    &vs->force_controls.slider, slide_target, vs->force_controls.force);
    }

    return true;
}


bool Event_channel_slide_force_length_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->force_slide_length, &value->value.Tstamp_type);

    Slider_set_length(&ch->force_controls.slider, &ch->force_slide_length);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_set_length(&vs->force_controls.slider, &ch->force_slide_length);
    }

    return true;
}


bool Event_channel_tremolo_speed_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    ch->tremolo_speed = value->value.float_type;

    LFO_set_speed(&ch->force_controls.tremolo, ch->tremolo_speed);

    if (ch->tremolo_depth > 0)
        LFO_set_depth(&ch->force_controls.tremolo, ch->tremolo_depth);

    LFO_turn_on(&ch->force_controls.tremolo);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        LFO_set_speed(&vs->force_controls.tremolo, value->value.float_type);

        if (ch->tremolo_depth > 0)
            LFO_set_depth(&vs->force_controls.tremolo, ch->tremolo_depth);

        LFO_turn_on(&vs->force_controls.tremolo);
    }

    return true;
}


bool Event_channel_tremolo_depth_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const double actual_depth = value->value.float_type / 6;
    ch->tremolo_depth = actual_depth;

    if (ch->tremolo_speed > 0)
        LFO_set_speed(&ch->force_controls.tremolo, ch->tremolo_speed);

    LFO_set_depth(&ch->force_controls.tremolo, actual_depth);
    LFO_turn_on(&ch->force_controls.tremolo);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (ch->tremolo_speed > 0)
            LFO_set_speed(&vs->force_controls.tremolo, ch->tremolo_speed);

        LFO_set_depth(&vs->force_controls.tremolo, actual_depth);
        LFO_turn_on(&vs->force_controls.tremolo);
    }

    return true;
}


bool Event_channel_tremolo_speed_slide_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->tremolo_speed_slide, &value->value.Tstamp_type);

    LFO_set_speed_slide(&ch->force_controls.tremolo, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_speed_slide(&vs->force_controls.tremolo, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_tremolo_depth_slide_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->tremolo_depth_slide, &value->value.Tstamp_type);

    LFO_set_depth_slide(&ch->force_controls.tremolo, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_depth_slide(&vs->force_controls.tremolo, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_carry_force_on_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    ch->carry_force = true;

    return true;
}


bool Event_channel_carry_force_off_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    ch->carry_force = false;

    return true;
}


