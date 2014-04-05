

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
#include <float.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <module/Scale.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_common.h>
#include <player/Voice.h>
#include <Value.h>


bool Event_channel_slide_pitch_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice* voice = ch->fg[i];
        if (voice->gen->ins_params->pitch_locks[i].enabled)
            continue;

        Voice_state* vs = voice->state;
        pitch_t pitch = -1;
#if 0
        if (voice->gen->ins_params->scale == NULL ||
                *voice->gen->ins_params->scale == NULL ||
                **voice->gen->ins_params->scale == NULL)
#endif
        {
            pitch = exp2(value->value.float_type / 1200) * 440;
        }
#if 0
        else
        {
            pitch = Scale_get_pitch_from_cents(
                    **voice->gen->ins_params->scale,
                    value->value.float_type);
        }
#endif
        if (pitch <= 0)
            continue;

        if (Slider_in_progress(&vs->pitch_slider))
            Slider_change_target(&vs->pitch_slider, pitch);
        else
            Slider_start(&vs->pitch_slider, pitch, vs->pitch);
    }

    return true;
}


bool Event_channel_slide_pitch_length_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->pitch_slide_length, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        Slider_set_length(&vs->pitch_slider, &value->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_vibrato_speed_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    ch->vibrato_speed = value->value.float_type;
    LFO_set_speed(&ch->vibrato, value->value.float_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        if (ch->fg[i]->gen->ins_params->pitch_locks[i].enabled)
            continue;

        Voice_state* vs = ch->fg[i]->state;
        LFO_set_speed(&vs->vibrato, value->value.float_type);
        if (ch->vibrato_depth > 0)
            LFO_set_depth(&vs->vibrato, ch->vibrato_depth);

        LFO_turn_on(&vs->vibrato);
    }

    return true;
}


bool Event_channel_vibrato_depth_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    double actual_depth = value->value.float_type / 240; // unit is 5 cents
    ch->vibrato_depth = actual_depth;
    LFO_set_depth(&ch->vibrato, actual_depth); // unit is 5 cents

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        if (ch->fg[i]->gen->ins_params->pitch_locks[i].enabled)
            continue;

        Voice_state* vs = ch->fg[i]->state;
        if (ch->vibrato_speed > 0)
            LFO_set_speed(&vs->vibrato, ch->vibrato_speed);

        LFO_set_depth(&vs->vibrato, actual_depth);
        LFO_turn_on(&vs->vibrato);
    }

    return true;
}


bool Event_channel_vibrato_delay_process(
        Channel* ch,
        Device_states* dstates,
        Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    (void)dstates;
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->vibrato_depth_delay,
                 &value->value.Tstamp_type);
    LFO_set_depth_delay(&ch->vibrato, &value->value.Tstamp_type);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;
        LFO_set_depth_delay(&vs->vibrato, &value->value.Tstamp_type);
    }

    return true;
}


