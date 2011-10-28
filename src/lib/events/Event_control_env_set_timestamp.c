

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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
#include <limits.h>

#include <Env_var.h>
#include <Environment.h>
#include <Event_control_env_set_timestamp.h>
#include <Event_type.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <Playdata.h>
#include <Reltime.h>
#include <xassert.h>


static Event_field_desc env_set_timestamp_desc[] =
{
    {
        .type = EVENT_FIELD_STRING
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.Reltime_type = { INT64_MIN, 0 },
        .max.field.Reltime_type = { INT64_MAX, KQT_RELTIME_BEAT - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


bool Event_control_env_set_timestamp_process(General_state* gstate,
                                             char* fields)
{
    assert(gstate != NULL);
    if (fields == NULL || !gstate->global)
    {
        return false;
    }
    Event_field data[2];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, env_set_timestamp_desc, data, state);
    if (state->error)
    {
        return false;
    }
    char var_name[ENV_VAR_NAME_MAX] = "";
    state = READ_STATE_AUTO;
    read_string(data[0].field.string_type, var_name, ENV_VAR_NAME_MAX, state);
    if (state->error)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    Environment* env = global_state->env;
    Env_var* var = Environment_get(env, var_name);
    if (var == NULL || Env_var_get_type(var) != ENV_VAR_RELTIME)
    {
        return false;
    }
    Env_var_modify_value(var, &data[1].field.Reltime_type);
    return true;
}


