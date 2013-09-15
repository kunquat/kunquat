

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
#include <kunquat/limits.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


struct Env_var
{
    char name[ENV_VAR_NAME_MAX];
    Value value;
};


static bool is_valid_name(const char* name)
{
    assert(name != NULL);

    return strlen(name) < ENV_VAR_NAME_MAX &&
        strspn(name, ENV_VAR_CHARS) == strlen(name) &&
        strchr(ENV_VAR_INIT_CHARS, name[0]) != NULL;
}


Env_var* new_Env_var(const char* name)
{
    assert(name != NULL);
    assert(is_valid_name(name));

    Env_var* var = memory_alloc_item(Env_var);
    if (var == NULL)
        return NULL;

    strcpy(var->name, name);
    var->value.type = VALUE_TYPE_NONE;

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
    char type_name[16] = "";
    char name[ENV_VAR_NAME_MAX] = "";
    *str = read_const_char(*str, '[', state);
    *str = read_string(*str, type_name, 16, state);
    *str = read_const_char(*str, ',', state);
    *str = read_string(*str, name, ENV_VAR_NAME_MAX, state);
    *str = read_const_char(*str, ',', state);
    if (state->error)
    {
        return NULL;
    }

    if (!is_valid_name(name))
    {
        Read_state_set_error(state, "Illegal variable name %s"
                             " (Variable names may only contain"
                             " lower-case letters and underscores"
                             " (and digits as other than first characters))",
                             name);
        return NULL;
    }

    Value* value = VALUE_AUTO;

    if (string_eq(type_name, "bool"))
    {
        value->type = VALUE_TYPE_BOOL;
        *str = read_bool(*str, &value->value.bool_type, state);
    }
    else if (string_eq(type_name, "int"))
    {
        value->type = VALUE_TYPE_INT;
        *str = read_int(*str, &value->value.int_type, state);
    }
    else if (string_eq(type_name, "float"))
    {
        value->type = VALUE_TYPE_FLOAT;
        *str = read_double(*str, &value->value.float_type, state);
    }
#if 0
    else if (string_eq(type_name, "real"))
    {
        value->type = VALUE_TYPE_REAL;
        *str = read_tuning(*str, &value->value.Real_type, NULL, state);
    }
#endif
    else if (string_eq(type_name, "timestamp"))
    {
        value->type = VALUE_TYPE_TSTAMP;
        *str = read_tstamp(*str, &value->value.Tstamp_type, state);
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

    Env_var* var = new_Env_var(name);
    if (var == NULL)
        return NULL;

    Env_var_set_value(var, value);

    return var;
}


Value_type Env_var_get_type(const Env_var* var)
{
    assert(var != NULL);
    return var->value.type;
}


const char* Env_var_get_name(const Env_var* var)
{
    assert(var != NULL);
    return var->name;
}


void Env_var_set_value(Env_var* var, const Value* value)
{
    assert(var != NULL);
    assert(value != NULL);
    assert(var->value.type == value->type);

    Value_copy(&var->value, value);

    return;
}


const Value* Env_var_get_value(const Env_var* var)
{
    assert(var != NULL);
    return &var->value;
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


