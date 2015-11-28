

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
#include <module/Module.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/set_active_name.h>
#include <Value.h>


static bool try_update_cv(Channel* ch, const Value* value, Active_type active_type)
{
    assert(ch != NULL);
    assert(value != NULL);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, active_type);
    if (var_name == NULL)
        return false;

    const int32_t au_index =
        Module_get_au_index_from_input(ch->parent.module, ch->au_input);
    if (au_index < 0)
        return false;

    const bool was_value_set =
        Channel_cv_state_set_value(ch->cvstate, au_index, var_name, value);

    return was_value_set;
}


static void set_cv_value_generic(Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);

    static const Active_type active_types[VALUE_TYPE_COUNT] =
    {
        [VALUE_TYPE_BOOL]   = ACTIVE_TYPE_BOOL,
        [VALUE_TYPE_INT]    = ACTIVE_TYPE_INT,
        [VALUE_TYPE_FLOAT]  = ACTIVE_TYPE_FLOAT,
        [VALUE_TYPE_TSTAMP] = ACTIVE_TYPE_TSTAMP,
    };

    const Active_type active_type = active_types[value->type];

    if (!try_update_cv(ch, value, active_type))
        return;

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, active_type);
    assert(var_name != NULL);

    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    assert(au != NULL);

    const Device* dev = (const Device*)au;
    Device_set_control_var_generic(
            dev, dstates, DEVICE_CONTROL_VAR_MODE_VOICE, ch, var_name, value);

    return;
}


bool Event_channel_set_cv_bool_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_BOOL, value);
}


bool Event_channel_set_cv_bool_value_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_BOOL);

    set_cv_value_generic(ch, dstates, value);

    return true;
}


bool Event_channel_set_cv_int_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_INT, value);
}


bool Event_channel_set_cv_int_value_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    set_cv_value_generic(ch, dstates, value);

    return true;
}


bool Event_channel_set_cv_float_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT, value);
}


bool Event_channel_set_cv_float_value_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    set_cv_value_generic(ch, dstates, value);

    return true;
}


bool Event_channel_slide_cv_float_target_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
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


bool Event_channel_slide_cv_float_length_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
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


bool Event_channel_osc_speed_cv_float_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
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


bool Event_channel_osc_depth_cv_float_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
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


bool Event_channel_osc_speed_slide_cv_float_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
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


bool Event_channel_osc_depth_slide_cv_float_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    const char* var_name = Active_names_get(
            ch->parent.active_names, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_FLOAT);
    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if ((var_name == NULL) || (au == NULL))
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


bool Event_channel_set_cv_tstamp_name_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    return set_active_name(
            &ch->parent, ACTIVE_CAT_AU_CONTROL_VAR, ACTIVE_TYPE_TSTAMP, value);
}


bool Event_channel_set_cv_tstamp_value_process(
        Channel* ch, Device_states* dstates, const Value* value)
{
    assert(ch != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    set_cv_value_generic(ch, dstates, value);

    return true;
}


