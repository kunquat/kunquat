

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_au_decl.h>

#include <init/devices/Au_streams.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_type.h>
#include <debug/assert.h>
#include <player/Channel.h>
#include <player/devices/processors/Stream_state.h>
#include <player/events/Event_params.h>
#include <player/events/set_active_name.h>
#include <string/var_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


static Device_state* get_target_dstate(
        const Audio_unit* au, Device_states* dstates, const char* stream_name)
{
    rassert(au != NULL);
    rassert(dstates != NULL);
    rassert(stream_name != NULL);

    const Au_streams* streams = Audio_unit_get_streams(au);
    if (streams == NULL)
        return NULL;

    const int proc_index = Au_streams_get_target_proc_index(streams, stream_name);
    if (proc_index < 0)
        return NULL;

    const Processor* proc = Audio_unit_get_proc(au, proc_index);
    if ((proc == NULL) || !Device_is_existent((const Device*)proc))
        return NULL;

    Device_state* dstate =
        Device_states_get_state(dstates, Device_get_id((const Device*)proc));
    if (Device_impl_get_proc_type(dstate->device->dimpl) != Proc_type_stream)
        return NULL;

    return dstate;
}


bool Event_au_set_stream_value_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_set_value(dstate, params->arg->value.float_type);

    return true;
}


bool Event_au_slide_stream_target_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_slide_target(dstate, params->arg->value.float_type);

    return true;
}


bool Event_au_slide_stream_length_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_slide_length(dstate, &params->arg->value.Tstamp_type);

    return true;
}


bool Event_au_stream_osc_speed_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_set_osc_speed(dstate, params->arg->value.float_type);

    return true;
}


bool Event_au_stream_osc_depth_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_set_osc_depth(dstate, params->arg->value.float_type);

    return true;
}


bool Event_au_stream_osc_speed_slide_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_set_osc_speed_slide(dstate, &params->arg->value.Tstamp_type);

    return true;
}


bool Event_au_stream_osc_depth_slide_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    const char* stream_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_STREAM);

    if ((stream_name == NULL) || !is_valid_var_name(stream_name))
        return false;

    Device_state* dstate = get_target_dstate(au, dstates, stream_name);
    if (dstate == NULL)
        return true;

    Stream_pstate_set_osc_depth_slide(dstate, &params->arg->value.Tstamp_type);

    return true;
}


