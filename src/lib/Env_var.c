

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
#include <stdint.h>
#include <string.h>

#include <Env_var.h>
#include <File_base.h>
#include <memory.h>
#include <Real.h>
#include <Reltime.h>
#include <serialise.h>
#include <string_common.h>
#include <xassert.h>


typedef union
{
    bool bool_type;
    int64_t int_type;
    double float_type;
    Real Real_type;
    Reltime Reltime_type;
} Container;


struct Env_var
{
    char name[ENV_VAR_NAME_MAX];
    Env_var_type type;
    Container data;
    Container initial;
};


static const size_t sizes[] =
{
    [ENV_VAR_BOOL] = sizeof(bool),
    [ENV_VAR_INT] = sizeof(int64_t),
    [ENV_VAR_FLOAT] = sizeof(double),
    [ENV_VAR_REAL] = sizeof(Real),
    [ENV_VAR_RELTIME] = sizeof(Reltime),
};


Env_var* new_Env_var(Env_var_type type, const char* name)
{
    assert(name != NULL);
    Env_var* var = memory_alloc_item(Env_var);
    if (var == NULL)
    {
        return NULL;
    }
    strncpy(var->name, name, ENV_VAR_NAME_MAX);
    var->name[ENV_VAR_NAME_MAX - 1] = '\0';
    var->type = type;
    switch (type)
    {
        case ENV_VAR_BOOL:
        {
            var->initial.bool_type = false;
        } break;
        case ENV_VAR_INT:
        {
            var->initial.int_type = 0;
        } break;
        case ENV_VAR_FLOAT:
        {
            var->initial.float_type = 0;
        } break;
        case ENV_VAR_REAL:
        {
            Real_init(&var->initial.Real_type);
        } break;
        case ENV_VAR_RELTIME:
        {
            Reltime_init(&var->initial.Reltime_type);
        } break;
        default:
            assert(false);
    }
    Env_var_reset(var);
    return var;
}


Env_var* new_Env_var_from_string(char** str, Read_state* state)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Env_var_type type = ENV_VAR_BOOL;
    char type_name[16] = "";
    char name[ENV_VAR_NAME_MAX] = "";
    Container data;
    *str = read_const_char(*str, '[', state);
    *str = read_string(*str, type_name, 16, state);
    *str = read_const_char(*str, ',', state);
    *str = read_string(*str, name, ENV_VAR_NAME_MAX, state);
    *str = read_const_char(*str, ',', state);
    if (state->error)
    {
        return NULL;
    }
    if (strspn(name, ENV_VAR_CHARS) != strlen(name) ||
            strchr(ENV_VAR_INIT_CHARS, name[0]) == NULL)
    {
        Read_state_set_error(state, "Illegal variable name %s"
                             " (Variable names may only contain"
                             " lower-case letters and underscores"
                             " (and digits as other than first characters))",
                             name);
        return NULL;
    }
    if (string_eq(type_name, "bool"))
    {
        type = ENV_VAR_BOOL;
        *str = read_bool(*str, &data.bool_type, state);
    }
    else if (string_eq(type_name, "int"))
    {
        type = ENV_VAR_INT;
        *str = read_int(*str, &data.int_type, state);
    }
    else if (string_eq(type_name, "float"))
    {
        type = ENV_VAR_FLOAT;
        *str = read_double(*str, &data.float_type, state);
    }
    else if (string_eq(type_name, "real"))
    {
        type = ENV_VAR_REAL;
        *str = read_tuning(*str, &data.Real_type, NULL, state);
    }
    else if (string_eq(type_name, "timestamp"))
    {
        type = ENV_VAR_RELTIME;
        *str = read_reltime(*str, &data.Reltime_type, state);
    }
    else
    {
        Read_state_set_error(state, "Invalid type of environment variable"
                                    " %s: %s", name, type_name);
        return NULL;
    }
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        return NULL;
    }
    Env_var* var = new_Env_var(type, name);
    if (var == NULL)
    {
        return NULL;
    }
    Env_var_set_value(var, &data);
    return var;
}


Env_var_type Env_var_get_type(Env_var* var)
{
    assert(var != NULL);
    return var->type;
}


char* Env_var_get_name(Env_var* var)
{
    assert(var != NULL);
    return var->name;
}


void Env_var_set_value(Env_var* var, void* value)
{
    assert(var != NULL);
    assert(value != NULL);
    memcpy(&var->initial, value, sizes[var->type]);
    Env_var_reset(var);
    return;
}


void Env_var_modify_value(Env_var* var, void* value)
{
    assert(var != NULL);
    assert(value != NULL);
    memcpy(&var->data, value, sizes[var->type]);
    return;
}


void Env_var_reset(Env_var* var)
{
    assert(var != NULL);
    memcpy(&var->data, &var->initial, sizes[var->type]);
    return;
}


void* Env_var_get_value(Env_var* var)
{
    assert(var != NULL);
    return &var->data;
}


void Env_var_get_value_json(Env_var* var,
                            char* dest,
                            int size)
{
    assert(var != NULL);
    assert(dest != NULL);
    assert(size > 0);
    switch (var->type)
    {
        case ENV_VAR_BOOL:
        {
            serialise_bool(dest, size, var->data.bool_type);
        } break;
        case ENV_VAR_INT:
        {
            serialise_int(dest, size, var->data.int_type);
        } break;
        case ENV_VAR_FLOAT:
        {
            serialise_float(dest, size, var->data.float_type);
        } break;
        case ENV_VAR_REAL:
        {
            serialise_Real(dest, size, &var->data.Real_type);
        } break;
        case ENV_VAR_RELTIME:
        {
            serialise_Timestamp(dest, size, &var->data.Reltime_type);
        } break;
        default:
            assert(false);
    }
    return;
}


void del_Env_var(Env_var* var)
{
    if (var == NULL)
    {
        return;
    }
    memory_free(var);
    return;
}


