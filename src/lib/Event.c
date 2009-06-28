

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <Event.h>

#include <Note_table.h>
#include <Song_limits.h>

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
    while (event->field_types[count].type != EVENT_FIELD_TYPE_NONE)
    {
        ++count;
    }
    return count;
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
        int field_count = Event_get_field_count(event);
        for (int i = 0; i < field_count; ++i)
        {
            int64_t numi = 0;
            double numd = NAN;
            Real* real = Real_init(REAL_AUTO);
            Reltime* rt = Reltime_init(RELTIME_AUTO);
            void* data = NULL;
            switch (event->field_types[i].type)
            {
                case EVENT_FIELD_TYPE_INT:
                case EVENT_FIELD_TYPE_NOTE:
                case EVENT_FIELD_TYPE_NOTE_MOD:
                {
                    str = read_int(str, &numi, state);
                    if (state->error)
                    {
                        return str;
                    }
                    data = &numi;
                }
                break;
                case EVENT_FIELD_TYPE_DOUBLE:
                {
                    str = read_double(str, &numd, state);
                    if (state->error)
                    {
                        return str;
                    }
                    data = &numd;
                }
                break;
                case EVENT_FIELD_TYPE_REAL:
                {
                    str = read_tuning(str, real, &numd, state);
                    if (state->error)
                    {
                        return str;
                    }
                    data = real;
                }
                break;
                case EVENT_FIELD_TYPE_RELTIME:
                {
                    str = read_reltime(str, rt, state);
                    if (state->error)
                    {
                        return str;
                    }
                    data = rt;
                }
                break;
                default:
                {
                    // Erroneous internal structures
                    assert(false);
                }
                break;
            }
            assert(data != NULL);
            if (!Event_set_field(event, i, data))
            {
                Read_state_set_error(state, "Field %d is not inside valid range.", i);
                return str;
            }
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
    }
    return str;
}


bool Event_write(Event* event, FILE* out, Write_state* state)
{
    assert(event != NULL);
    assert(out != NULL);
    assert(state != NULL);
    return false;
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


void* Event_get_field(Event* event, int index)
{
    assert(event != NULL);
    assert(event->get != NULL);
    return event->get(event, index);
}


bool Event_set_field(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->set != NULL);
    assert(data != NULL);
    return event->set(event, index, data);
}


void del_Event(Event* event)
{
    assert(event != NULL);
    assert(event->destroy != NULL);
    event->destroy(event);
    return;
}


