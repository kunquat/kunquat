

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
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
#include <stdio.h>
#include <math.h>

#include <Env_var.h>
#include <Event_names.h>
#include <File_base.h>
#include <math_common.h>
#include <Real.h>
#include <Reltime.h>
#include <serialise.h>
#include <Set_binding.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef union Value
{
    bool bool_type;
    int64_t int_type;
    double float_type;
    Real Real_type;
    Reltime Reltime_type;
} Value;


typedef struct Interval
{
    Value range[2];
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
    Target* cur_target;
    Value cur_value;
};


static bool Interval_parse(char** str,
                           Interval* in,
                           Env_var_type type,
                           bool allow_degen,
                           Read_state* state);


static int print_value(Env_var_type type,
                       Value* value,
                       char* dest_event,
                       int dest_size);


static int scale_from_float(double src_value,
                            Value* src_range,
                            Env_var_type dest_type,
                            Value* dest_range,
                            char* dest_event,
                            int dest_size);


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
    sb->cur_target = NULL;
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
        bool swap = Interval_parse(str, &t->src, sb->type, false, state);
        if (state->error)
        {
            del_Set_binding(sb);
            return NULL;
        }
        *str = read_const_char(*str, ',', state);
        int64_t channel = -2;
        *str = read_int(*str, &channel, state);
        *str = read_const_char(*str, ',', state);
        *str = read_string(*str, t->dest_name, EVENT_NAME_MAX, state);
        *str = read_const_char(*str, ',', state);
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
        if (Event_names_get(names, t->dest_name) == EVENT_NONE)
        {
            Read_state_set_error(state, "Target event name \"%s\" is"
                                        " not supported", t->dest_name);
            del_Set_binding(sb);
            return NULL;
        }
        t->dest_type = Event_names_get_param_type(names, t->dest_name);
        Interval_parse(str, &t->dest, t->dest_type, true, state);
        *str = read_const_char(*str, ']', state);
        if (state->error)
        {
            del_Set_binding(sb);
            return NULL;
        }
        t->channel = channel;
        if (swap)
        {
            Value tmp = t->src.range[0];
            t->src.range[0] = t->src.range[1];
            t->src.range[1] = tmp;
            tmp = t->dest.range[0];
            t->dest.range[0] = t->dest.range[1];
            t->dest.range[1] = tmp;
        }
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


bool Set_binding_get_first(Set_binding* sb,
                           char* field,
                           char* dest_event,
                           int dest_size,
                           int* channel)
{
    assert(sb != NULL);
    assert(field != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    assert(channel != NULL);
    Read_state* state = READ_STATE_AUTO;
    switch (sb->type)
    {
        case ENV_VAR_BOOL:
        {
            read_bool(field, &sb->cur_value.bool_type, state);
        } break;
        case ENV_VAR_INT:
        {
            read_int(field, &sb->cur_value.int_type, state);
        } break;
        case ENV_VAR_FLOAT:
        {
            read_double(field, &sb->cur_value.float_type, state);
        } break;
        case ENV_VAR_REAL:
        {
            read_tuning(field, &sb->cur_value.Real_type, NULL, state);
        } break;
        case ENV_VAR_RELTIME:
        {
            read_reltime(field, &sb->cur_value.Reltime_type, state);
        } break;
        default:
            assert(false);
    }
    assert(!state->error);
    sb->cur_target = sb->first;
    return Set_binding_get_next(sb, dest_event, dest_size, channel);
}


bool Set_binding_get_next(Set_binding* sb,
                          char* dest_event,
                          int dest_size,
                          int* channel)
{
    assert(sb != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    assert(channel != NULL);
    if (sb->cur_target == NULL)
    {
        return false;
    }
    Target* t = sb->cur_target;
    *channel = t->channel;
    int printed = snprintf(dest_event, dest_size, "[\"%s\", [",
                           t->dest_name);
    assert(printed < dest_size);
    dest_size -= printed;
    dest_event += printed;
    switch (sb->type)
    {
        case ENV_VAR_BOOL:
        {
            printed = print_value(t->dest_type,
                                  &t->dest.range[sb->cur_value.bool_type],
                                  dest_event, dest_size);
        } break;
        case ENV_VAR_INT:
        {
            Value range[2];
            range[0].float_type = t->src.range[0].int_type;
            range[1].float_type = t->src.range[1].int_type;
            printed = scale_from_float(sb->cur_value.int_type,
                                       range,
                                       t->dest_type,
                                       t->dest.range,
                                       dest_event,
                                       dest_size);
        } break;
        case ENV_VAR_FLOAT:
        {
            printed = scale_from_float(sb->cur_value.float_type,
                                       t->src.range,
                                       t->dest_type,
                                       t->dest.range,
                                       dest_event,
                                       dest_size);
        } break;
        case ENV_VAR_REAL:
        {
            assert(false);
        } break;
        case ENV_VAR_RELTIME:
        {
            assert(false);
        } break;
        default:
            assert(false);
    }
    assert(printed < dest_size);
    dest_size -= printed;
    dest_event += printed;
    snprintf(dest_event, dest_size, "]]");
    sb->cur_target = sb->cur_target->next;
    return true;
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


static int print_value(Env_var_type type,
                       Value* value,
                       char* dest_event,
                       int dest_size)
{
    assert(type > ENV_VAR_NONE);
    assert(type < ENV_VAR_LAST);
    assert(value != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    switch (type)
    {
        case ENV_VAR_BOOL:
        {
            return serialise_bool(dest_event, dest_size, value->bool_type);
        } break;
        case ENV_VAR_INT:
        {
            return serialise_int(dest_event, dest_size, value->int_type);
        } break;
        case ENV_VAR_FLOAT:
        {
            return serialise_float(dest_event, dest_size, value->float_type);
        } break;
        case ENV_VAR_REAL:
        {
            return serialise_Real(dest_event, dest_size, &value->Real_type);
        } break;
        case ENV_VAR_RELTIME:
        {
            return serialise_Timestamp(dest_event, dest_size,
                                       &value->Reltime_type);
        } break;
        default:
            assert(false);
    }
    assert(false);
    return 0;
}


static int scale_from_float(double src_value,
                            Value* src_range,
                            Env_var_type dest_type,
                            Value* dest_range,
                            char* dest_event,
                            int dest_size)
{
    assert(isfinite(src_value));
    assert(src_range != NULL);
    assert(dest_type < ENV_VAR_LAST);
    assert(dest_range != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    if (dest_type == ENV_VAR_BOOL)
    {
        return serialise_bool(dest_event, dest_size,
                    dest_range[fabs(src_value - src_range[0].float_type) <
                    fabs(src_value - src_range[1].float_type)].bool_type);
    }
    src_value = MAX(src_value, src_range[0].float_type);
    src_value = MIN(src_value, src_range[1].float_type);
    double dest_value = (src_value - src_range[0].float_type) /
                        (src_range[1].float_type - src_range[0].float_type);
    switch (dest_type)
    {
        case ENV_VAR_INT:
        {
            dest_value = dest_value *
                    (dest_range[1].int_type - dest_range[0].int_type) +
                    dest_range[0].int_type;
            return serialise_int(dest_event, dest_size, round(dest_value));
        } break;
        case ENV_VAR_FLOAT:
        {
            dest_value = dest_value *
                    (dest_range[1].float_type - dest_range[0].float_type) +
                    dest_range[0].float_type;
            return serialise_float(dest_event, dest_size, dest_value);
        } break;
        case ENV_VAR_REAL:
        {
            double dr0 = Real_get_double(&dest_range[0].Real_type);
            double dr1 = Real_get_double(&dest_range[1].Real_type);
            dest_value = dest_value * (dr1 - dr0) + dr0;
            Real* result = Real_init_as_double(REAL_AUTO, dest_value);
            return serialise_Real(dest_event, dest_size, result);
        } break;
        case ENV_VAR_RELTIME:
        {
            Reltime* rdiff = Reltime_sub(RELTIME_AUTO,
                                         &dest_range[1].Reltime_type,
                                         &dest_range[0].Reltime_type);
            double rdiffd = Reltime_get_beats(rdiff) +
                            Reltime_get_rem(rdiff) / (double)KQT_RELTIME_BEAT;
            double dr0 = Reltime_get_beats(&dest_range[0].Reltime_type) +
                         Reltime_get_rem(&dest_range[0].Reltime_type) /
                         (double)KQT_RELTIME_BEAT;
            dest_value = dest_value * rdiffd + dr0;
            Reltime* result = Reltime_set(RELTIME_AUTO, floor(dest_value),
                                          (dest_value - floor(dest_value)) *
                                          KQT_RELTIME_BEAT);
            return serialise_Timestamp(dest_event, dest_size, result);
        } break;
        default:
            assert(false);
    }
    assert(false);
    return 0;
}


static bool Interval_parse(char** str,
                           Interval* in,
                           Env_var_type type,
                           bool allow_degen,
                           Read_state* state)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(in != NULL);
    assert(type < ENV_VAR_LAST);
    assert(state != NULL);
    bool swap = false;
    const char* range_error = "Range endpoints must differ";
    *str = read_const_char(*str, '[', state);
    switch (type)
    {
        case ENV_VAR_BOOL:
        {
            *str = read_bool(*str, &in->range[0].bool_type, state);
            *str = read_const_char(*str, ',', state);
            *str = read_bool(*str, &in->range[1].bool_type, state);
            if (!state->error)
            {
                if (!allow_degen &&
                        in->range[0].bool_type == in->range[1].bool_type)
                {
                    Read_state_set_error(state, range_error);
                }
                else
                {
                    swap = in->range[0].bool_type > in->range[1].bool_type;
                }
            }
        } break;
        case ENV_VAR_INT:
        {
            *str = read_int(*str, &in->range[0].int_type, state);
            *str = read_const_char(*str, ',', state);
            *str = read_int(*str, &in->range[1].int_type, state);
            if (!state->error)
            {
                if (!allow_degen &&
                        in->range[0].int_type == in->range[1].int_type)
                {
                    Read_state_set_error(state, range_error);
                }
                else
                {
                    swap = in->range[0].int_type > in->range[1].int_type;
                }
            }
        } break;
        case ENV_VAR_FLOAT:
        {
            *str = read_double(*str, &in->range[0].float_type, state);
            *str = read_const_char(*str, ',', state);
            *str = read_double(*str, &in->range[1].float_type, state);
            if (!state->error)
            {
                if (!allow_degen &&
                        in->range[0].float_type == in->range[1].float_type)
                {
                    Read_state_set_error(state, range_error);
                }
                else
                {
                    swap = in->range[0].float_type > in->range[1].float_type;
                }
            }
        } break;
        case ENV_VAR_REAL:
        {
            *str = read_tuning(*str, &in->range[0].Real_type, NULL, state);
            *str = read_const_char(*str, ',', state);
            *str = read_tuning(*str, &in->range[1].Real_type, NULL, state);
            if (!state->error)
            {
                if (!allow_degen && Real_cmp(&in->range[0].Real_type,
                                             &in->range[1].Real_type) == 0)
                {
                    Read_state_set_error(state, range_error);
                }
                else
                {
                    swap = Real_cmp(&in->range[0].Real_type,
                                    &in->range[1].Real_type) > 0;
                }
            }
        } break;
        case ENV_VAR_RELTIME:
        {
            *str = read_reltime(*str, &in->range[0].Reltime_type, state);
            *str = read_const_char(*str, ',', state);
            *str = read_reltime(*str, &in->range[1].Reltime_type, state);
            if (!state->error)
            {
                if (!allow_degen && Reltime_cmp(&in->range[0].Reltime_type,
                                            &in->range[1].Reltime_type) == 0)
                {
                    Read_state_set_error(state, range_error);
                }
                else
                {
                    swap = Reltime_cmp(&in->range[0].Reltime_type,
                                       &in->range[1].Reltime_type) > 0;
                }
            }
        } break;
        default:
            assert(false);
    }
    *str = read_const_char(*str, ']', state);
    return swap;
}


