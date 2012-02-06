

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Event.h>
#include <Scale.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


Event_field_desc* Event_get_field_types(Event* event)
{
    assert(event != NULL);
    assert(event->field_types != NULL);
    return event->field_types;
}


int Event_get_field_count(Event* event)
{
    assert(event != NULL);
    assert(event->field_types != NULL);
    int count = 0;
    while (event->field_types[count].type != EVENT_FIELD_NONE)
    {
        ++count;
    }
    return count;
}


char* Event_type_get_fields(char* str,
                            Event_field_desc field_descs[],
                            Event_field fields[],
                            Read_state* state)
{
    assert(str != NULL);
    assert(field_descs != NULL);
    assert(state != NULL);
    //str = read_const_char(str, '[', state);
    if (state->error)
    {
        return str;
    }
    //const char* error_message = "Event argument is not inside valid range";
    switch (field_descs[0].type)
    {
        case EVENT_FIELD_NONE:
        {
            str = read_null(str, state);
        } break;
        case EVENT_FIELD_BOOL:
        case EVENT_FIELD_INT:
        case EVENT_FIELD_DOUBLE:
        case EVENT_FIELD_REAL:
        case EVENT_FIELD_RELTIME:
        case EVENT_FIELD_STRING:
        {
            char* str_pos = str;
            str = read_string(str, NULL, 0, state);
            if (state->error)
            {
                return str;
            }
            if (fields != NULL)
            {
                fields[0].field.string_type = str_pos;
            }
        } break;
#if 0
        case EVENT_FIELD_BOOL:
        {
            bool* value = fields != NULL ?
                              &fields[0].field.bool_type : NULL;
            str = read_bool(str, value, state);
            if (state->error)
            {
                return str;
            }
        } break;
        case EVENT_FIELD_INT:
        {
            int64_t num = 0;
            str = read_int(str, &num, state);
            if (state->error)
            {
                return str;
            }
            if (num < field_descs[0].min.field.integral_type ||
                    num > field_descs[0].max.field.integral_type)
            {
                Read_state_set_error(state, error_message);
                return str;
            }
            if (fields != NULL)
            {
                fields[0].field.integral_type = num;
            }
        } break;
        case EVENT_FIELD_DOUBLE:
        {
            double num = NAN;
            str = read_double(str, &num, state);
            if (state->error)
            {
                return str;
            }
            if (num < field_descs[0].min.field.double_type ||
                    num > field_descs[0].max.field.double_type)
            {
                Read_state_set_error(state, error_message);
                return str;
            }
            if (fields != NULL)
            {
                fields[0].field.double_type = num;
            }
        } break;
        case EVENT_FIELD_REAL:
        {
            Real* value = fields != NULL ?
                              &fields[0].field.Real_type : NULL;
            double numd = NAN;
            str = read_tuning(str, value, &numd, state);
            if (state->error)
            {
                return str;
            }
        } break;
        case EVENT_FIELD_RELTIME:
        {
            Reltime* rt = RELTIME_AUTO;
            str = read_reltime(str, rt, state);
            if (state->error)
            {
                return str;
            }
            if (Reltime_cmp(rt, &field_descs[0].min.field.Reltime_type) < 0 ||
                    Reltime_cmp(rt, &field_descs[0].max.field.Reltime_type) > 0)
            {
                Read_state_set_error(state, error_message);
                return str;
            }
            if (fields != NULL)
            {
                Reltime_copy(&fields[0].field.Reltime_type, rt);
            }
        } break;
        case EVENT_FIELD_STRING:
        {
            char* str_pos = str;
            str = read_string(str, NULL, 0, state);
            if (state->error)
            {
                return str;
            }
            if (fields != NULL)
            {
                fields[0].field.string_type = str_pos;
            }
        } break;
#endif
        default:
            assert(false);
    }
#if 0
    for (int i = 0; field_descs[i].type != EVENT_FIELD_NONE; ++i)
    {
        if (i > 0)
        {
            str = read_const_char(str, ',', state);
        }
        const char* error_message = "Event field %d is not inside valid range";
        switch (field_descs[i].type)
        {
            case EVENT_FIELD_BOOL:
            {
                bool* value = fields != NULL ?
                                  &fields[i].field.bool_type : NULL;
                str = read_bool(str, value, state);
                if (state->error)
                {
                    return str;
                }
            }
            break;
            case EVENT_FIELD_INT:
            //case EVENT_FIELD_NOTE:
            //case EVENT_FIELD_NOTE_MOD:
            {
                int64_t num = 0;
                str = read_int(str, &num, state);
                if (state->error)
                {
                    return str;
                }
                if (num < field_descs[i].min.field.integral_type ||
                        num > field_descs[i].max.field.integral_type)
                {
                    Read_state_set_error(state, error_message, i);
                    return str;
                }
                if (fields != NULL)
                {
                    fields[i].field.integral_type = num;
                }
            }
            break;
            case EVENT_FIELD_DOUBLE:
            {
                double num = NAN;
                str = read_double(str, &num, state);
                if (state->error)
                {
                    return str;
                }
                if (num < field_descs[i].min.field.double_type ||
                        num > field_descs[i].max.field.double_type)
                {
                    Read_state_set_error(state, error_message, i);
                    return str;
                }
                if (fields != NULL)
                {
                    fields[i].field.double_type = num;
                }
            }
            break;
            case EVENT_FIELD_REAL:
            {
                Real* value = fields != NULL ?
                                  &fields[i].field.Real_type : NULL;
                double numd = NAN;
                str = read_tuning(str, value, &numd, state);
                if (state->error)
                {
                    return str;
                }
            }
            break;
            case EVENT_FIELD_RELTIME:
            {
                Reltime* rt = RELTIME_AUTO;
                str = read_reltime(str, rt, state);
                if (state->error)
                {
                    return str;
                }
                if (Reltime_cmp(rt, &field_descs[i].min.field.Reltime_type) < 0 ||
                        Reltime_cmp(rt, &field_descs[i].max.field.Reltime_type) > 0)
                {
                    Read_state_set_error(state, error_message, i);
                    return str;
                }
                if (fields != NULL)
                {
                    Reltime_copy(&fields[i].field.Reltime_type, rt);
                }
            }
            break;
            case EVENT_FIELD_STRING:
            {
                char* str_pos = str;
                str = read_string(str, NULL, 0, state);
                if (state->error)
                {
                    return str;
                }
                if (fields != NULL)
                {
                    fields[i].field.string_type = str_pos;
                }
            }
            break;
            default:
            {
                assert(false);
            }
            break;
        }
    }
#endif
    //str = read_const_char(str, ']', state);
    return str;
}


char* Event_read(Event* event, char* str, Read_state* state)
{
    assert(event != NULL);
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    char* fields_start = str /*- 1*/;
    char* fields_end = Event_type_get_fields(fields_start,
                                             event->field_types,
                                             NULL, state);
    if (state->error)
    {
        return fields_end;
    }
    assert(fields_end != NULL);
    assert(fields_end > fields_start);
    if (Event_get_field_count(event) > 0)
    {
        event->fields = xcalloc(char, fields_end - fields_start + 1);
        if (event->fields == NULL)
        {
            Read_state_set_error(state, "Couldn't allocate memory.");
            return fields_end;
        }
        strncpy(event->fields, fields_start, fields_end - fields_start);
        return fields_end;
    }
    return fields_end;
}


Reltime* Event_get_pos(Event* event)
{
    assert(event != NULL);
    return &event->pos;
}


void Event_set_pos(Event* event, Reltime* pos)
{
    assert(event != NULL);
    assert(pos != NULL);
    Reltime_copy(&event->pos, pos);
    return;
}


Event_type Event_get_type(Event* event)
{
    assert(event != NULL);
    return event->type;
}


char* Event_get_desc(Event* event)
{
    assert(event != NULL);
    return event->desc;
}


char* Event_get_fields(Event* event)
{
    assert(event != NULL);
    return event->fields;
}


void del_Event(Event* event)
{
    if (event == NULL)
    {
        return;
    }
    assert(event->destroy != NULL);
    event->destroy(event);
    return;
}


