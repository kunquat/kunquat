

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdbool.h>
#include <stdlib.h>

#include <debug/assert.h>
#include <player/events/Event_au_decl.h>
#include <player/events/set_active_name.h>
#include <Value.h>


bool Event_au_set_cv_bool_value_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_BOOL);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_BOOL);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            master_params->random,
            NULL,
            var_name,
            value);

    return true;
}


bool Event_au_set_cv_int_value_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_INT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            master_params->random,
            NULL,
            var_name,
            value);

    return true;
}


bool Event_au_set_cv_float_value_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            master_params->random,
            NULL,
            var_name,
            value);

    return true;
}


bool Event_au_slide_cv_float_target_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_slide_control_var_float_target(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            channel,
            var_name,
            value->value.float_type);

    return true;
}


bool Event_au_slide_cv_float_length_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_slide_control_var_float_length(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            channel,
            var_name,
            &value->value.Tstamp_type);

    return true;
}


bool Event_au_osc_speed_cv_float_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_speed_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            channel,
            var_name,
            value->value.float_type);

    return true;
}


bool Event_au_osc_depth_cv_float_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_depth_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            channel,
            var_name,
            value->value.float_type);

    return true;
}


bool Event_au_osc_speed_slide_cv_float_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_speed_slide_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            channel,
            var_name,
            &value->value.Tstamp_type);

    return true;
}


bool Event_au_osc_depth_slide_cv_float_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_FLOAT);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_depth_slide_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            channel,
            var_name,
            &value->value.Tstamp_type);

    return true;
}


bool Event_au_set_cv_tstamp_value_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(master_params != NULL);
    assert(channel != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            channel->parent.active_names, ACTIVE_CAT_CONTROL_VAR, ACTIVE_TYPE_TSTAMP);

    if (var_name == NULL)
        return true;

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_MIXED,
            master_params->random,
            channel,
            var_name,
            value);

    return true;
}


