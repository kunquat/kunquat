

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
#include <stdint.h>

#include <Env_var.h>
#include <Event_names.h>
#include <Real.h>
#include <Reltime.h>
#include <Set_binding.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef union Interval
{
    bool bool_type[2];
    int64_t int_type[2];
    double float_type[2];
    Real Real_type[2];
    Reltime Reltime_type[2];
} Interval;


typedef struct Target
{
    Env_var_type src_type;
    Interval src;
    char dest_name[EVENT_NAME_MAX + 1];
    Env_var_type dest_type;
    Interval dest;
    int channel;
    struct Target* next;
} Target;


struct Set_binding
{
    char name[ENV_VAR_NAME_MAX];
    Env_var_type type;
    Target* first;
};


static bool Interval_parse(char** str,
                           Interval* in,
                           Env_var_type type,
                           Read_state* state);


Set_binding* new_Set_binding_from_string(char** str,
                                         Event_names* names,
                                         Read_state* state)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(names != NULL);
    assert(state != NULL);
    Set_binding* sb = xalloc(Set_binding);
    if (sb == NULL)
    {
        return NULL;
    }
    sb->first = NULL;
    *str = read_const_char(*str, '[', state);
    char type_name[16] = "";
    *str = read_string(*str, type_name, 16, state);
    *str = read_const_char(*str, ',', state);
    *str = read_string(*str, sb->name, ENV_VAR_NAME_MAX, state);
    *str = read_const_char(*str, ',', state);
    if (string_eq(type_name, "bool"))
    {
        sb->type = ENV_VAR_BOOL;
    }
    else if (string_eq(type_name, "int"))
    {
        sb->type = ENV_VAR_INT;
    }
    else if (string_eq(type_name, "float"))
    {
        sb->type = ENV_VAR_FLOAT;
    }
    else if (string_eq(type_name, "real"))
    {
        sb->type = ENV_VAR_REAL;
    }
    else if (string_eq(type_name, "timestamp"))
    {
        sb->type = ENV_VAR_RELTIME;
    }
    else
    {
        Read_state_set_error(state, "Invalid type of environment variable"
                                    " %s: %s", sb->name, type_name);
        return NULL;
    }

    *str = read_const_char(*str, '[', state);
    if (state->error)
    {
        del_Set_binding(sb);
        return NULL;
    }
    *str = read_const_char(*str, ']', state);
    if (!state->error)
    {
        *str = read_const_char(*str, ']', state);
        if (state->error)
        {
            del_Set_binding(sb);
            return NULL;
        }
        return sb;
    }
    Read_state_clear_error(state);
    bool expect_entry = true;
    Target* last = NULL;
    while (expect_entry)
    {
        *str = read_const_char(*str, '[', state);
        Target* t = xalloc(Target);
        if (t == NULL || state->error)
        {
            xfree(t);
            del_Set_binding(sb);
            return NULL;
        }
        t->next = NULL;
        if (last == NULL)
        {
            assert(sb->first == NULL);
            sb->first = last = t;
        }
        else
        {
            last->next = t;
            last = t;
        }
        Interval_parse(str, &t->src, sb->type, state);
        if (state->error)
        {
            del_Set_binding(sb);
            return NULL;
        }
        *str = read_const_char(*str, ',', state);
        *str = read_string(*str, t->dest_name, EVENT_NAME_MAX, state);
        *str = read_const_char(*str, ',', state);
        if (state->error)
        {
            del_Set_binding(sb);
            return NULL;
        }
        if (Event_names_get(names, t->dest_name) == EVENT_NONE)
        {
            Read_state_set_error(state, "Target event name \"%s\" is"
                                        " not supported", t->dest_name);
            del_Set_binding(sb);
            return NULL;
        }
        t->dest_type = Event_names_get_param_type(names, t->dest_name);
        Interval_parse(str, &t->dest, t->dest_type, state);
        *str = read_const_char(*str, ',', state);
        int64_t channel = -2;
        *str = read_int(*str, &channel, state);
        *str = read_const_char(*str, ']', state);
        if (state->error)
        {
            del_Set_binding(sb);
            return NULL;
        }
        if (channel < -1 || channel >= KQT_COLUMNS_MAX)
        {
            Read_state_set_error(state, "Invalid channel number: %d",
                                        (int)channel);
            del_Set_binding(sb);
            return NULL;
        }
        t->channel = channel;
        check_next(*str, state, expect_entry);
    }
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Set_binding(sb);
        return NULL;
    }
    return sb;
}


void del_Set_binding(Set_binding* sb)
{
    if (sb == NULL)
    {
        return;
    }
    Target* t = sb->first;
    while (t != NULL)
    {
        Target* next = t->next;
        xfree(t);
        t = next;
    }
    xfree(sb);
    return;
}


static bool Interval_parse(char** str,
                           Interval* in,
                           Env_var_type type,
                           Read_state* state)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(in != NULL);
    assert(type < ENV_VAR_LAST);
    assert(state != NULL);
    const char* range_error = "Range endpoints must differ";
    *str = read_const_char(*str, '[', state);
    switch (type)
    {
        case ENV_VAR_BOOL:
        {
            *str = read_bool(*str, &in->bool_type[0], state);
            *str = read_const_char(*str, ',', state);
            *str = read_bool(*str, &in->bool_type[1], state);
            if (!state->error && in->bool_type[0] == in->bool_type[1])
            {
                Read_state_set_error(state, range_error);
            }
        } break;
        case ENV_VAR_INT:
        {
            *str = read_int(*str, &in->int_type[0], state);
            *str = read_const_char(*str, ',', state);
            *str = read_int(*str, &in->int_type[1], state);
            if (!state->error && in->int_type[0] == in->int_type[1])
            {
                Read_state_set_error(state, range_error);
            }
        } break;
        case ENV_VAR_FLOAT:
        {
            *str = read_double(*str, &in->float_type[0], state);
            *str = read_const_char(*str, ',', state);
            *str = read_double(*str, &in->float_type[1], state);
            if (!state->error && in->float_type[0] == in->float_type[1])
            {
                Read_state_set_error(state, range_error);
            }
        } break;
        case ENV_VAR_REAL:
        {
            *str = read_tuning(*str, &in->Real_type[0], NULL, state);
            *str = read_const_char(*str, ',', state);
            *str = read_tuning(*str, &in->Real_type[1], NULL, state);
            if (!state->error && Real_cmp(&in->Real_type[0],
                                          &in->Real_type[1]) == 0)
            {
                Read_state_set_error(state, range_error);
            }
        } break;
        case ENV_VAR_RELTIME:
        {
            *str = read_reltime(*str, &in->Reltime_type[0], state);
            *str = read_const_char(*str, ',', state);
            *str = read_reltime(*str, &in->Reltime_type[1], state);
            if (!state->error && Reltime_cmp(&in->Reltime_type[0],
                                             &in->Reltime_type[1]) == 0)
            {
                Read_state_set_error(state, range_error);
            }
        } break;
        default:
            assert(false);
    }
    *str = read_const_char(*str, ']', state);
    return !state->error;
}


