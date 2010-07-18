

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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
    assert(fields != NULL);
    assert(state != NULL);
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        return str;
    }
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
                str = read_bool(str, &fields[i].field.bool_type, state);
                if (state->error)
                {
                    return str;
                }
            }
            break;
            case EVENT_FIELD_INT:
            case EVENT_FIELD_NOTE:
            case EVENT_FIELD_NOTE_MOD:
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
                fields[i].field.integral_type = num;
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
                fields[i].field.double_type = num;
            }
            break;
            case EVENT_FIELD_REAL:
            {
                double numd = NAN;
                str = read_tuning(str, &fields[i].field.Real_type,
                                  &numd, state);
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
                Reltime_copy(&fields[i].field.Reltime_type, rt);
            }
            break;
            case EVENT_FIELD_STRING:
            {
                str = read_string(str, NULL, 0, state);
                if (state->error)
                {
                    return str;
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
    str = read_const_char(str, ']', state);
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
    char* fields_start = NULL;
    char* fields_end = NULL;
    int event_count = Event_get_field_count(event);
    if (event_count > 0)
    {
        str = read_const_char(str, ',', state);
        if (state->error)
        {
            return str;
        }
        str = read_const_char(str, '[', state);
        if (state->error)
        {
            return str;
        }
        fields_start = str - 1;
        int field_count = Event_get_field_count(event);
        for (int i = 0; i < field_count; ++i)
        {
#if 0
            bool bool_value = false;
            int64_t numi = 0;
            double numd = NAN;
            Real* real = Real_init(REAL_AUTO);
            Reltime* rt = Reltime_init(RELTIME_AUTO);
            void* data = NULL;
#endif
            switch (event->field_types[i].type)
            {
                case EVENT_FIELD_BOOL:
                {
                    str = read_bool(str, NULL, state);
#if 0
                    if (state->error)
                    {
                        return str;
                    }
                    data = &bool_value;
#endif
                }
                break;
                case EVENT_FIELD_INT:
                case EVENT_FIELD_NOTE:
                case EVENT_FIELD_NOTE_MOD:
                {
                    str = read_int(str, NULL, state);
#if 0
                    if (state->error)
                    {
                        return str;
                    }
                    data = &numi;
#endif
                }
                break;
                case EVENT_FIELD_DOUBLE:
                {
                    str = read_double(str, NULL, state);
#if 0
                    if (state->error)
                    {
                        return str;
                    }
                    data = &numd;
#endif
                }
                break;
                case EVENT_FIELD_REAL:
                {
                    str = read_tuning(str, NULL, NULL, state);
#if 0
                    if (state->error)
                    {
                        return str;
                    }
                    data = real;
#endif
                }
                break;
                case EVENT_FIELD_RELTIME:
                {
                    str = read_reltime(str, NULL, state);
#if 0
                    if (state->error)
                    {
                        return str;
                    }
                    data = rt;
#endif
                }
                break;
                case EVENT_FIELD_STRING:
                {
                    str = read_string(str, NULL, 0, state);
#if 0
                    if (state->error)
                    {
                        return str;
                    }
#endif
                }
                break;
                default:
                {
                    // Erroneous internal structures
                    assert(false);
                }
                break;
            }
#if 0
            assert(data != NULL || event->field_types[i].type == EVENT_FIELD_STRING);
            if (!Event_set_field(event, i, data))
            {
                Read_state_set_error(state, "Field %d is not inside valid range.", i);
                return str;
            }
#endif
            if (i < field_count - 1)
            {
                str = read_const_char(str, ',', state);
                if (state->error)
                {
                    return str;
                }
            }
        }
        str = read_const_char(str, ']', state);
        if (state->error)
        {
            return str;
        }
        fields_end = str;
    }
    if (fields_start != NULL)
    {
        Event_field data[16]; // FIXME: limit
        Event_type_get_fields(fields_start, event->field_types, data, state);
        if (state->error)
        {
            return str;
        }
        assert(fields_end != NULL);
        assert(fields_end >= fields_start);
        event->fields = xcalloc(char, fields_end - fields_start + 1);
        if (event->fields == NULL)
        {
            Read_state_set_error(state, "Couldn't allocate memory.");
            return str;
        }
        strncpy(event->fields, fields_start, fields_end - fields_start);
    }
    return str;
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


#if 0
void* Event_get_field(Event* event, int index)
{
    assert(false);
    assert(event != NULL);
    assert(event->get != NULL);
    return event->get(event, index);
}


bool Event_set_field(Event* event, int index, void* data)
{
    assert(false);
    assert(event != NULL);
    assert(event->set != NULL);
    assert(data != NULL || event->field_types[index].type == EVENT_FIELD_STRING);
    if (event->field_types[index].type == EVENT_FIELD_STRING)
    {
        return true;
    }
    return event->set(event, index, data);
}
#endif


char* Event_get_fields(Event* event)
{
    assert(event != NULL);
    return event->fields;
}


void del_Event(Event* event)
{
    assert(event != NULL);
    assert(event->destroy != NULL);
    event->destroy(event);
    return;
}


