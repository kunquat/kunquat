

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
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
#include <init/devices/Proc_type.h>
#include <init/Module.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>
#include <player/devices/processors/Force_state.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/Force_controls.h>
#include <player/Master_params.h>
#include <player/Voice.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


static Force_controls* get_force_controls(Voice_state* vstate)
{
    if (vstate->proc_type != Proc_type_force)
        return NULL;

    return Force_vstate_get_force_controls_mut(vstate);
}


bool Event_channel_set_force_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const double force_shift = master_params->parent.module->force_shift;

    const double force = params->arg->value.float_type + force_shift;

    ch->force_controls.force = force;
    Slider_break(&ch->force_controls.slider);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
        {
            fc->force = force;
            Slider_break(&fc->slider);
        }
    }

    return true;
}


bool Event_channel_slide_force_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const double force_shift = master_params->parent.module->force_shift;

    const double slide_target = params->arg->value.float_type + force_shift;
    const double start_force =
        isfinite(ch->force_controls.force) ? ch->force_controls.force : slide_target;

    if (Slider_in_progress(&ch->force_controls.slider))
        Slider_change_target(&ch->force_controls.slider, slide_target);
    else
        Slider_start(&ch->force_controls.slider, slide_target, start_force);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
        {
            if (Slider_in_progress(&fc->slider))
                Slider_change_target(&fc->slider, slide_target);
            else
                Slider_start(&fc->slider, slide_target, fc->force);
        }
    }

    return true;
}


bool Event_channel_slide_force_length_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->force_slide_length, &params->arg->value.Tstamp_type);

    Slider_set_length(&ch->force_controls.slider, &ch->force_slide_length);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
            Slider_set_length(&fc->slider, &ch->force_slide_length);
    }

    return true;
}


bool Event_channel_tremolo_speed_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    ch->tremolo_speed = params->arg->value.float_type;

    LFO_set_speed(&ch->force_controls.tremolo, ch->tremolo_speed);

    if (ch->tremolo_depth > 0)
        LFO_set_depth(&ch->force_controls.tremolo, ch->tremolo_depth);

    LFO_turn_on(&ch->force_controls.tremolo);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
        {
            LFO_set_speed(&fc->tremolo, params->arg->value.float_type);

            if (ch->tremolo_depth > 0)
                LFO_set_depth(&fc->tremolo, ch->tremolo_depth);

            LFO_turn_on(&fc->tremolo);
        }
    }

    return true;
}


bool Event_channel_tremolo_depth_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const double actual_depth = params->arg->value.float_type;
    ch->tremolo_depth = actual_depth;

    if (ch->tremolo_speed > 0)
        LFO_set_speed(&ch->force_controls.tremolo, ch->tremolo_speed);

    LFO_set_depth(&ch->force_controls.tremolo, actual_depth);
    LFO_turn_on(&ch->force_controls.tremolo);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
        {
            if (ch->tremolo_speed > 0)
                LFO_set_speed(&fc->tremolo, ch->tremolo_speed);

            LFO_set_depth(&fc->tremolo, actual_depth);
            LFO_turn_on(&fc->tremolo);
        }
    }

    return true;
}


bool Event_channel_tremolo_speed_slide_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->tremolo_speed_slide, &params->arg->value.Tstamp_type);

    LFO_set_speed_slide(&ch->force_controls.tremolo, &params->arg->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
            LFO_set_speed_slide(&fc->tremolo, &params->arg->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_tremolo_depth_slide_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    Tstamp_copy(&ch->tremolo_depth_slide, &params->arg->value.Tstamp_type);

    LFO_set_depth_slide(&ch->force_controls.tremolo, &params->arg->value.Tstamp_type);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        Event_check_voice(ch, i);
        Voice_state* vs = ch->fg[i]->state;

        Force_controls* fc = get_force_controls(vs);
        if (fc != NULL)
            LFO_set_depth_slide(&fc->tremolo, &params->arg->value.Tstamp_type);
    }

    return true;
}


bool Event_channel_carry_force_on_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    ch->carry_force = true;

    return true;
}


bool Event_channel_carry_force_off_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    ch->carry_force = false;

    return true;
}


