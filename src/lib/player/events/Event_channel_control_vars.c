

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
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
#include <player/Channel.h>
#include <player/Channel_cv_state.h>
#include <player/events/Event_params.h>
#include <player/events/set_active_name.h>
#include <Value.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


static bool try_update_cv(Channel* ch, const Value* value)
{
    rassert(ch != NULL);
    rassert(value != NULL);

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
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(value != NULL);

    if (!try_update_cv(ch, value))
        return;

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    rassert(var_name != NULL);

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
    rassert(ch != NULL);
    rassert(dstates != NULL);

    const char* var_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_CONTROL_VAR);
    if (var_name == NULL)
        return;

    Channel_cv_state_set_carrying_enabled(ch->cvstate, var_name, enabled);

    return;
}


bool Event_channel_set_cv_name_process(
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
    rassert(params->arg->type == VALUE_TYPE_STRING);

    return set_active_name(&ch->parent, ACTIVE_CAT_CONTROL_VAR, params->arg);
}


bool Event_channel_set_cv_value_process(
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
    rassert(Value_type_is_realtime(params->arg->type));

    set_cv_value_generic(ch, dstates, params->arg);

    return true;
}


bool Event_channel_carry_cv_on_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    set_cv_carry(ch, dstates, true);

    return true;
}


bool Event_channel_carry_cv_off_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    ignore(params);

    set_cv_carry(ch, dstates, false);

    return true;
}


