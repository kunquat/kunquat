

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


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <init/Module.h>
#include <player/events/set_active_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


static bool try_update_cv(Channel* ch, const Value* value)
{
    assert(ch != NULL);
    assert(value != NULL);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    if (var_name == NULL)
        return false;

    const bool was_value_set =
        Channel_cv_state_set_value(ch->cvstate, var_name, value);

    return was_value_set;
}


static void set_cv_value_generic(Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);

    //const Active_type active_type = active_types[value->type];

    if (!try_update_cv(ch, value))
        return;

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    assert(var_name != NULL);

    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if (au == NULL)
        return;

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            Channel_get_random_source(ch),
            ch,
            var_name,
            value);

    return;
}


static void set_cv_carry(Channel* ch, Device_states* dstates, bool enabled)
{
    assert(ch != NULL);
    assert(dstates != NULL);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    if (var_name == NULL)
        return;

    Channel_cv_state_set_carrying_enabled(ch->cvstate, var_name, enabled);

    return;
}


bool Event_channel_set_cv_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(&ch->parent, ACTIVE_CAT_CONTROL_VAR, value);
}


bool Event_channel_set_cv_value_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(Value_type_is_realtime(value->type));

    set_cv_value_generic(ch, dstates, value);

    return true;
}


bool Event_channel_carry_cv_on_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    set_cv_carry(ch, dstates, true);

    return true;
}


bool Event_channel_carry_cv_off_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    ignore(value);

    set_cv_carry(ch, dstates, false);

    return true;
}


bool Event_channel_slide_cv_target_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
        return true;

    if (!Channel_cv_state_slide_target_float(
                ch->cvstate, var_name, value->value.float_type))
        return true;

    const Device* dev = (const Device*)au;
    Device_slide_control_var_float_target(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            ch,
            var_name,
            value->value.float_type);

    return true;
}


bool Event_channel_slide_cv_length_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
        return true;

    if (!Channel_cv_state_slide_length_float(
                ch->cvstate, var_name, &value->value.Tstamp_type))
        return true;

    const Device* dev = (const Device*)au;
    Device_slide_control_var_float_length(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            ch,
            var_name,
            &value->value.Tstamp_type);

    return true;
}


bool Event_channel_osc_speed_cv_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
        return true;

    if (!Channel_cv_state_osc_speed_float(
                ch->cvstate, var_name, value->value.float_type))
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_speed_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            ch,
            var_name,
            value->value.float_type);

    return true;
}


bool Event_channel_osc_depth_cv_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
        return true;

    if (!Channel_cv_state_osc_depth_float(
                ch->cvstate, var_name, value->value.float_type))
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_depth_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            ch,
            var_name,
            value->value.float_type);

    return true;
}


bool Event_channel_osc_speed_slide_cv_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
        return true;

    if (!Channel_cv_state_osc_speed_slide_float(
                ch->cvstate, var_name, &value->value.Tstamp_type))
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_speed_slide_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            ch,
            var_name,
            &value->value.Tstamp_type);

    return true;
}


bool Event_channel_osc_depth_slide_cv_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
        return true;

    if (!Channel_cv_state_osc_depth_slide_float(
                ch->cvstate, var_name, &value->value.Tstamp_type))
        return true;

    const Device* dev = (const Device*)au;
    Device_osc_depth_slide_cv_float(
            dev,
            dstates,
            DEVICE_CONTROL_VAR_MODE_VOICE,
            ch,
            var_name,
            &value->value.Tstamp_type);

    return true;
}


