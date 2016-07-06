

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
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
#include <player/Channel_stream_state.h>
#include <player/devices/processors/Stream_state.h>
#include <player/events/set_active_name.h>
#include <player/events/stream_utils.h>
#include <string/var_name.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


bool Event_channel_set_stream_name_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_STRING);

    return set_active_name(&channel->parent, ACTIVE_CAT_STREAM, value);
}


bool Event_channel_set_stream_value_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    if (!Channel_stream_state_set_value(ss, stream_name, value->value.float_type))
        return true;

    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);
    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


static void ensure_valid_stream(
        Channel_stream_state* ss, const char* stream_name, const Voice_state* vstate)
{
    rassert(ss != NULL);
    rassert(stream_name != NULL);

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    if (controls == NULL)
        return;

    if (isnan(Linear_controls_get_value(controls)))
    {
        if (vstate != NULL)
            Channel_stream_state_set_controls(
                    ss, stream_name, Stream_vstate_get_controls(vstate));
        else
            Channel_stream_state_set_value(ss, stream_name, 0);
    }

    return;
}


bool Event_channel_slide_stream_target_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);

    ensure_valid_stream(ss, stream_name, vstate);

    if (!Channel_stream_state_slide_target(ss, stream_name, value->value.float_type))
        return true;

    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


bool Event_channel_slide_stream_length_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);

    ensure_valid_stream(ss, stream_name, vstate);

    if (!Channel_stream_state_slide_length(ss, stream_name, &value->value.Tstamp_type))
        return true;

    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


bool Event_channel_stream_osc_speed_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);

    ensure_valid_stream(ss, stream_name, vstate);

    if (!Channel_stream_state_set_osc_speed(ss, stream_name, value->value.float_type))
        return true;

    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


bool Event_channel_stream_osc_depth_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);

    ensure_valid_stream(ss, stream_name, vstate);

    if (!Channel_stream_state_set_osc_depth(ss, stream_name, value->value.float_type))
        return true;

    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


bool Event_channel_stream_osc_speed_slide_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);

    ensure_valid_stream(ss, stream_name, vstate);

    if (!Channel_stream_state_set_osc_speed_slide(
                ss, stream_name, &value->value.Tstamp_type))
        return true;

    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


bool Event_channel_stream_osc_depth_slide_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    Voice_state* vstate = get_target_stream_vstate(channel, stream_name);

    ensure_valid_stream(ss, stream_name, vstate);

    if (!Channel_stream_state_set_osc_depth_slide(
                ss, stream_name, &value->value.Tstamp_type))
        return true;

    if (vstate == NULL)
        return true;

    const Linear_controls* controls = Channel_stream_state_get_controls(ss, stream_name);
    Stream_vstate_set_controls(vstate, controls);

    return true;
}


bool Event_channel_carry_stream_on_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    return Channel_stream_state_set_carrying_enabled(ss, stream_name, true);
}


bool Event_channel_carry_stream_off_process(
        Channel* channel,
        Device_states* dstates,
        const Master_params* master_params,
        const Value* value)
{
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(value);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Channel_stream_state* ss = Channel_get_stream_state_mut(channel);
    return Channel_stream_state_set_carrying_enabled(ss, stream_name, false);
}


