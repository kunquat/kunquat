

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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

#include <Active_names.h>
#include <Env_var.h>
#include <Environment.h>
#include <Event.h>
#include <Event_common.h>
#include <Event_control_decl.h>
#include <Event_type.h>
#include <File_base.h>
#include <General_state.h>
#include <set_active_name.h>
#include <transient/Master_params.h>
#include <Value.h>
#include <xassert.h>


bool Event_control_env_set_bool_process(General_state* mgstate, General_state* gstate, Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_BOOL || !gstate->global)
    {
        return false;
    }
    Env_var* var = Environment_get(
            gstate->env,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_BOOL));
    if (var == NULL || Env_var_get_type(var) != ENV_VAR_BOOL)
    {
        return true;
    }
    Env_var_modify_value(var, &value->value.bool_type);
    return true;
}


bool Event_control_env_set_bool_name_process(
        General_state* mgstate,
        General_state* gstate,
        Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_STRING || !gstate->global)
    {
        return false;
    }
    return set_active_name(gstate, ACTIVE_CAT_ENV, ACTIVE_TYPE_BOOL, value);
}


bool Event_control_env_set_float_process(General_state* mgstate, General_state* gstate, Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT || !gstate->global)
    {
        return false;
    }
    Env_var* var = Environment_get(
            gstate->env,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_FLOAT));
    if (var == NULL || Env_var_get_type(var) != ENV_VAR_FLOAT)
    {
        return true;
    }
    Env_var_modify_value(var, &value->value.float_type);
    return true;
}


bool Event_control_env_set_float_name_process(
        General_state* mgstate,
        General_state* gstate,
        Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_STRING || !gstate->global)
    {
        return false;
    }
    return set_active_name(gstate, ACTIVE_CAT_ENV, ACTIVE_TYPE_FLOAT, value);
}


bool Event_control_env_set_int_process(General_state* mgstate, General_state* gstate, Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT || !gstate->global)
    {
        return false;
    }
    Env_var* var = Environment_get(
            gstate->env,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_INT));
    if (var == NULL || Env_var_get_type(var) != ENV_VAR_INT)
    {
        return true;
    }
    Env_var_modify_value(var, &value->value.int_type);
    return true;
}


bool Event_control_env_set_int_name_process(
        General_state* mgstate,
        General_state* gstate,
        Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_STRING || !gstate->global)
    {
        return false;
    }
    return set_active_name(gstate, ACTIVE_CAT_ENV, ACTIVE_TYPE_INT, value);
}


bool Event_control_env_set_tstamp_process(
        General_state* mgstate,
        General_state* gstate,
        Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP || !gstate->global)
    {
        return false;
    }
    Env_var* var = Environment_get(
            gstate->env,
            Active_names_get(
                gstate->active_names,
                ACTIVE_CAT_ENV,
                ACTIVE_TYPE_FLOAT));
    if (var == NULL || Env_var_get_type(var) != ENV_VAR_TSTAMP)
    {
        return true;
    }
    Env_var_modify_value(var, &value->value.Tstamp_type);
    return true;
}


bool Event_control_env_set_tstamp_name_process(
        General_state* mgstate,
        General_state* gstate,
        Value* value)
{
    assert(mgstate != NULL || gstate != NULL);
    if (gstate == NULL)
        gstate = mgstate;
    assert(value != NULL);
    if (value->type != VALUE_TYPE_STRING || !gstate->global)
    {
        return false;
    }
    return set_active_name(
            gstate,
            ACTIVE_CAT_ENV,
            ACTIVE_TYPE_TSTAMP,
            value);
}


