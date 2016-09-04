

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
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
#include <init/Module.h>
#include <init/Tuning_table.h>
#include <kunquat/limits.h>
#include <player/devices/processors/Pitch_state.h>
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/Master_params.h>
#include <player/Tuning_state.h>
#include <player/Voice.h>
#include <Value.h>

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


bool Event_channel_slide_pitch_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    double pitch = value->value.float_type;

    // Retune pitch parameter if a retuner is active
    {
        const int tuning_index = master_params->cur_tuning_state;
        if (0 <= tuning_index && tuning_index < KQT_TUNING_TABLES_MAX)
        {
            Tuning_state* state = master_params->tuning_states[tuning_index];
            const Tuning_table* table =
                Module_get_tuning_table(master_params->parent.module, tuning_index);
            if (state != NULL && table != NULL)
                pitch = Tuning_state_get_retuned_pitch(state, table, pitch);
        }
    }

    const double start_pitch =
        isfinite(ch->pitch_controls.pitch) ? ch->pitch_controls.pitch : pitch;

    if (Slider_in_progress(&ch->pitch_controls.slider))
        Slider_change_target(&ch->pitch_controls.slider, pitch);
    else
        Slider_start(&ch->pitch_controls.slider, pitch, start_pitch);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice* voice = ch->fg[i];

        Voice_state* vs = voice->state;

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);
    }

    return true;
}


bool Event_channel_slide_pitch_length_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->pitch_slide_length, &value->value.Tstamp_type);

    Slider_set_length(&ch->pitch_controls.slider, &ch->pitch_slide_length);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);
    }

    return true;
}


bool Event_channel_vibrato_speed_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    ch->vibrato_speed = value->value.float_type;

    LFO_set_speed(&ch->pitch_controls.vibrato, ch->vibrato_speed);
    if (ch->vibrato_depth > 0)
        LFO_set_depth(&ch->pitch_controls.vibrato, ch->vibrato_depth);

    LFO_turn_on(&ch->pitch_controls.vibrato);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);

        Voice_state* vs = ch->fg[i]->state;

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);
    }

    return true;
}


bool Event_channel_vibrato_depth_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    const double actual_depth = value->value.float_type * 5; // unit is 5 cents
    ch->vibrato_depth = actual_depth;

    if (ch->vibrato_speed > 0)
        LFO_set_speed(&ch->pitch_controls.vibrato, ch->vibrato_speed);

    LFO_set_depth(&ch->pitch_controls.vibrato, actual_depth);
    LFO_turn_on(&ch->pitch_controls.vibrato);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);

        Voice_state* vs = ch->fg[i]->state;

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);
    }

    return true;
}


bool Event_channel_vibrato_speed_slide_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->vibrato_speed_slide, &value->value.Tstamp_type);

    LFO_set_speed_slide(&ch->pitch_controls.vibrato, &ch->vibrato_speed_slide);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);
    }

    return true;
}


bool Event_channel_vibrato_depth_slide_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->vibrato_depth_slide, &value->value.Tstamp_type);

    LFO_set_depth_slide(&ch->pitch_controls.vibrato, &ch->vibrato_depth_slide);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        if (vs->is_pitch_state)
            Pitch_vstate_set_controls(vs, &ch->pitch_controls);
    }

    return true;
}


bool Event_channel_carry_pitch_on_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    ch->carry_pitch = true;

    return true;
}


bool Event_channel_carry_pitch_off_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    ch->carry_pitch = false;

    return true;
}


