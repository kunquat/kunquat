

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <string.h>

#include <Event.h>
#include <Event_names.h>
#include <Event_type.h>
#include <memory.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>


static void del_Event_default(Event* event);


Event* new_Event(Event_type type, Tstamp* pos)
{
    assert(Event_is_valid(type));
    assert(pos != NULL);

    Event* event = memory_alloc_item(Event);
    if (event == NULL)
        return NULL;

    event->type = type;
    Tstamp_copy(&event->pos, pos);
    event->desc = NULL;
    event->destroy = del_Event_default;

    return event;
}


Event* new_Event_from_string(
        char** str,
        Read_state* state,
        const Event_names* names)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    assert(names != NULL);

    if (state->error)
        return NULL;

    *str = read_const_char(*str, '[', state);
    Tstamp* pos = Tstamp_init(TSTAMP_AUTO);
    *str = read_tstamp(*str, pos, state);
    *str = read_const_char(*str, ',', state);
    *str = read_const_char(*str, '[', state);
    char* event_desc = *str - 1;
    char type_str[EVENT_NAME_MAX + 2] = "";
    *str = read_string(*str, type_str, EVENT_NAME_MAX + 2, state);
    *str = read_const_char(*str, ',', state);
    if (state->error)
        return NULL;

    Event_type type = Event_names_get(names, type_str);
    if (!Event_is_trigger(type))
    {
        Read_state_set_error(
                state,
                "Invalid or unsupported event type: \"%s\"",
                type_str);
        return NULL;
    }

    Event* event = new_Event(type, pos);
    if (event == NULL)
        return NULL;

    Value_type field_type = VALUE_TYPE_NONE;
    field_type = Event_names_get_param_type(names, type_str);

    if (field_type == VALUE_TYPE_NONE)
        *str = read_null(*str, state);
    else
        *str = read_string(*str, NULL, 0, state);
    if (state->error)
    {
        del_Event(event);
        return NULL;
    }

    assert(*str != NULL);
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Event(event);
        return NULL;
    }

    event->desc = memory_calloc_items(char, *str - event_desc + 1);
    if (event->desc == NULL)
    {
        del_Event(event);
        return NULL;
    }

    strncpy(event->desc, event_desc, *str - event_desc);
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Event(event);
        return NULL;
    }

    return event;
}


static void del_Event_default(Event* event)
{
    if (event == NULL)
        return;

    assert(Event_is_valid(event->type));
    memory_free(event->desc);
    memory_free(event);

    return;
}


