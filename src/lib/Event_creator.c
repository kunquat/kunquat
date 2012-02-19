

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
#include <stdbool.h>
#include <string.h>

#include <Event.h>
#include <Event_names.h>
#include <Event_type.h>
#include <Event_global_jump.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


static void del_Event_default(Event* event);


Event* new_Event(Event_type type, Reltime* pos)
{
    assert(EVENT_IS_VALID(type));
    assert(pos != NULL);
    Event* event = xalloc(Event);
    if (event == NULL)
    {
        return NULL;
    }
    event->type = type;
    Reltime_copy(&event->pos, pos);
    event->desc = NULL;
    event->destroy = del_Event_default;
    return event;
}


Event* new_Event_from_string(char** str, Read_state* state,
                             Event_names* names)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    assert(names != NULL);
    if (state->error)
    {
        return NULL;
    }
    *str = read_const_char(*str, '[', state);
    Reltime* pos = Reltime_init(RELTIME_AUTO);
    *str = read_reltime(*str, pos, state);
    *str = read_const_char(*str, ',', state);
    *str = read_const_char(*str, '[', state);
    char* event_desc = *str - 1;
    char type_str[EVENT_NAME_MAX + 2] = "";
    *str = read_string(*str, type_str, EVENT_NAME_MAX + 2, state);
    *str = read_const_char(*str, ',', state);
    if (state->error)
    {
        return NULL;
    }
    Event_type type = Event_names_get(names, type_str);
    if (!EVENT_IS_TRIGGER(type))
    {
        Read_state_set_error(state, "Invalid or unsupported event type:"
                                    " \"%s\"", type_str);
        return NULL;
    }
    Event* event = NULL;
    if (string_eq(type_str, "wj"))
    {
        event = new_Event_global_jump(pos);
    }
    else
    {
        event = new_Event(type, pos);
    }
    if (event == NULL)
    {
        return NULL;
    }
    Event_field_type field_type = EVENT_FIELD_NONE;
    if (!string_eq(type_str, "wj"))
    {
        field_type = Event_names_get_param_type(names, type_str);
    }
    if (field_type == EVENT_FIELD_NONE)
    {
        *str = read_null(*str, state);
    }
    else
    {
        *str = read_string(*str, NULL, 0, state);
    }
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
    event->desc = xcalloc(char, *str - event_desc + 1);
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
    {
        return;
    }
    assert(EVENT_IS_VALID(event->type));
    xfree(event->desc);
    xfree(event);
    return;
}


