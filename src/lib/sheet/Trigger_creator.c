

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <memory.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <sheet/Trigger.h>
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


Event* new_Event_from_string(Streader* sr, const Event_names* names)
{
    assert(sr != NULL);
    assert(names != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    // Trigger position
    Tstamp* pos = TSTAMP_AUTO;
    if (!Streader_readf(sr, "[%t,[", pos))
        return NULL;

    // Store event description location for copying
    const char* event_desc = &sr->str[sr->pos - 1];

    // Event type
    char type_str[EVENT_NAME_MAX + 2] = "";
    if (!(Streader_read_string(sr, EVENT_NAME_MAX + 2, type_str) &&
                Streader_match_char(sr, ',')))
        return NULL;

    Event_type type = Event_names_get(names, type_str);
    if (!Event_is_trigger(type))
    {
        Streader_set_error(
                sr, "Invalid or unsupported event type: \"%s\"", type_str);
        return NULL;
    }

    // Event argument
    Value_type field_type = VALUE_TYPE_NONE;
    field_type = Event_names_get_param_type(names, type_str);

    if (field_type == VALUE_TYPE_NONE)
        Streader_read_null(sr);
    else
        Streader_read_string(sr, 0, NULL);
    if (Streader_is_error_set(sr))
        return NULL;

    // End of event description
    Streader_match_char(sr, ']');
    if (Streader_is_error_set(sr))
        return NULL;

    // Create the trigger
    Event* event = new_Event(type, pos);
    if (event == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for a trigger");
        return NULL;
    }

    // Copy the event description
    event->desc = memory_calloc_items(
            char, (&sr->str[sr->pos] - event_desc) + 1);
    if (event->desc == NULL)
    {
        del_Event(event);
        return NULL;
    }

    strncpy(event->desc, event_desc, &sr->str[sr->pos] - event_desc);

    // End of trigger
    Streader_match_char(sr, ']');
    if (Streader_is_error_set(sr))
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


