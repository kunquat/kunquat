

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
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

#include <debug/assert.h>
#include <Env_var.h>
#include <Environment.h>
#include <player/Active_names.h>
#include <player/Event_type.h>
#include <player/events/Event_common.h>
#include <player/events/Event_control_decl.h>
#include <player/events/set_active_name.h>
#include <player/General_state.h>
#include <Value.h>


bool Event_control_env_set_bool_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_BOOL);

    if (!gstate->global)
        return false;

    Env_var* var = Env_state_get_var(
            gstate->estate,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_BOOL));
    if (var == NULL || Env_var_get_type(var) != VALUE_TYPE_BOOL)
        return true;

    Env_var_set_value(var, value);
    return true;
}


bool Event_control_env_set_bool_name_process(
        General_state* gstate,
        Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    if (!gstate->global)
        return false;

    return set_active_name(gstate, ACTIVE_CAT_ENV, ACTIVE_TYPE_BOOL, value);
}


bool Event_control_env_set_float_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);

    if (!gstate->global)
        return false;

    Env_var* var = Env_state_get_var(
            gstate->estate,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_FLOAT));
    if (var == NULL || Env_var_get_type(var) != VALUE_TYPE_FLOAT)
        return true;

    Env_var_set_value(var, value);
    return true;
}


bool Event_control_env_set_float_name_process(
        General_state* gstate,
        Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    if (!gstate->global)
        return false;

    return set_active_name(gstate, ACTIVE_CAT_ENV, ACTIVE_TYPE_FLOAT, value);
}


bool Event_control_env_set_int_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_INT);

    if (!gstate->global)
        return false;

    Env_var* var = Env_state_get_var(
            gstate->estate,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_INT));
    if (var == NULL || Env_var_get_type(var) != VALUE_TYPE_INT)
        return true;

    Env_var_set_value(var, value);
    return true;
}


bool Event_control_env_set_int_name_process(
        General_state* gstate,
        Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    if (!gstate->global)
        return false;

    return set_active_name(gstate, ACTIVE_CAT_ENV, ACTIVE_TYPE_INT, value);
}


bool Event_control_env_set_tstamp_process(
        General_state* gstate,
        Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_TSTAMP);

    if (!gstate->global)
        return false;

    Env_var* var = Env_state_get_var(
            gstate->estate,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_FLOAT));
    if (var == NULL || Env_var_get_type(var) != VALUE_TYPE_TSTAMP)
        return true;

    Env_var_set_value(var, value);
    return true;
}


bool Event_control_env_set_tstamp_name_process(
        General_state* gstate,
        Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    if (!gstate->global)
        return false;

    return set_active_name(
            gstate,
            ACTIVE_CAT_ENV,
            ACTIVE_TYPE_TSTAMP,
            value);
}


