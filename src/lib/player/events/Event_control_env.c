

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_control_decl.h>

#include <debug/assert.h>
#include <init/Env_var.h>
#include <init/Environment.h>
#include <player/Active_names.h>
#include <player/Channel.h>
#include <player/Event_type.h>
#include <player/events/Event_common.h>
#include <player/events/set_active_name.h>
#include <player/General_state.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_control_env_set_var_name_process(
        General_state* global_state, Channel* channel, const Value* value)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_STRING);

    return set_active_name(&channel->parent, ACTIVE_CAT_ENV, value);
}


bool Event_control_env_set_var_process(
        General_state* global_state, Channel* channel, const Value* value)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(value != NULL);

    Env_var* var = Env_state_get_var(
            global_state->estate,
            Active_names_get(channel->parent.active_names, ACTIVE_CAT_ENV));
    if (var == NULL)
        return false;

    Value* converted = VALUE_AUTO;
    if (!Value_convert(converted, value, Env_var_get_type(var)))
        return false;

    Env_var_set_value(var, converted);
    return true;
}


