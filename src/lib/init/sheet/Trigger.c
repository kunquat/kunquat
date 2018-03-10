

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/sheet/Trigger.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>

#include <stdlib.h>
#include <string.h>


Trigger* new_Trigger(Event_type type, Tstamp* pos)
{
    rassert(Event_is_valid(type));
    rassert(pos != NULL);

    Trigger* trigger = memory_alloc_item(Trigger);
    if (trigger == NULL)
        return NULL;

    trigger->type = type;
    Tstamp_copy(&trigger->pos, pos);
    trigger->desc = NULL;

    return trigger;
}


Trigger* new_Trigger_from_string(Streader* sr, const Event_names* names)
{
    rassert(sr != NULL);
    rassert(names != NULL);

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
    {
        Streader_read_null(sr);
    }
    else if ((field_type == VALUE_TYPE_MAYBE_STRING) ||
            (field_type == VALUE_TYPE_MAYBE_REALTIME))
    {
        if (!Streader_read_null(sr))
        {
            Streader_clear_error(sr);
            Streader_read_string(sr, 0, NULL);
        }
    }
    else
    {
        Streader_read_string(sr, 0, NULL);
    }
    if (Streader_is_error_set(sr))
        return NULL;

    // End of event description
    Streader_match_char(sr, ']');
    if (Streader_is_error_set(sr))
        return NULL;

    // Create the trigger
    Trigger* trigger = new_Trigger(type, pos);
    if (trigger == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for a trigger");
        return NULL;
    }

    // Copy the event description
    trigger->desc = memory_calloc_items(
            char, (&sr->str[sr->pos] - event_desc) + 1);
    if (trigger->desc == NULL)
    {
        del_Trigger(trigger);
        return NULL;
    }

    strncpy(trigger->desc, event_desc, (size_t)(&sr->str[sr->pos] - event_desc));

    // End of trigger
    Streader_match_char(sr, ']');
    if (Streader_is_error_set(sr))
    {
        del_Trigger(trigger);
        return NULL;
    }

    return trigger;
}


const Tstamp* Trigger_get_pos(const Trigger* trigger)
{
    rassert(trigger != NULL);
    return &trigger->pos;
}


Event_type Trigger_get_type(const Trigger* trigger)
{
    rassert(trigger != NULL);
    return trigger->type;
}


const char* Trigger_get_desc(const Trigger* trigger)
{
    rassert(trigger != NULL);
    return trigger->desc;
}


void del_Trigger(Trigger* trigger)
{
    if (trigger == NULL)
        return;

    rassert(Event_is_valid(trigger->type));
    memory_free(trigger->desc);
    memory_free(trigger);

    return;
}


