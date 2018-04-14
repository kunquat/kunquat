

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
#include <string/Streader.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NAME_SPEC_SEP ':'


bool Trigger_data_contains_name_spec(Streader* sr)
{
    rassert(sr != NULL);

    if (!Streader_readf(sr, "[%t,[", NULL))
        return false;

    char type_str[KQT_TRIGGER_NAME_MAX + 2] = "";
    if (!Streader_read_string(sr, KQT_TRIGGER_NAME_MAX + 2, type_str))
        return false;

    return (strchr(type_str, NAME_SPEC_SEP) != NULL);
}


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
    Streader_skip_whitespace(sr);
    const char* event_desc_from_name = &sr->str[sr->pos];
    const int event_name_offset = (int)(event_desc_from_name - event_desc);

    // Event type
    char type_str[KQT_TRIGGER_NAME_MAX + 2] = "";
    if (!(Streader_read_string(sr, KQT_TRIGGER_NAME_MAX + 2, type_str) &&
                Streader_match_char(sr, ',')))
        return NULL;

    // Remove name specifier
    char* sep_pos = strchr(type_str, NAME_SPEC_SEP);
    if (sep_pos != NULL)
        *sep_pos = '\0';

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
        Streader_set_memory_error(
                sr, "Could not allocate memory for a trigger");
        del_Trigger(trigger);
        return NULL;
    }

    strncpy(trigger->desc, event_desc, (size_t)(&sr->str[sr->pos] - event_desc));

    if (sep_pos != NULL)
    {
        // Remove the name specifier part
        const int event_name_length = (int)strlen(type_str);
        char* cut_pos = trigger->desc + event_name_offset + event_name_length + 1;
        rassert(*cut_pos == NAME_SPEC_SEP);
        char* name_end_pos = strchr(cut_pos, '"');
        rassert(name_end_pos != NULL);
        const char* desc_end = trigger->desc + strlen(trigger->desc);
        rassert(desc_end > name_end_pos);
        memmove(cut_pos, name_end_pos, (size_t)(desc_end - name_end_pos + 1));
    }

    // End of trigger
    Streader_match_char(sr, ']');
    if (Streader_is_error_set(sr))
    {
        del_Trigger(trigger);
        return NULL;
    }

    return trigger;
}


Trigger* new_Trigger_of_name_spec_from_string(Streader* sr, const Event_names* names)
{
    rassert(sr != NULL);
    rassert(names != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    // Trigger position
    Tstamp* pos = TSTAMP_AUTO;
    if (!Streader_readf(sr, "[%t,[", pos))
        return NULL;

    // Event type
    char type_str[KQT_TRIGGER_NAME_MAX + 2] = "";
    if (!(Streader_read_string(sr, KQT_TRIGGER_NAME_MAX + 2, type_str) &&
                Streader_match_char(sr, ',')))
        return NULL;

    // Separate name specifier
    char* sep_pos = strchr(type_str, NAME_SPEC_SEP);
    rassert(sep_pos != NULL);
    *sep_pos = '\0';
    const char* name_arg = sep_pos + 1;
    const size_t name_length = strlen(name_arg);
    if (name_length == 0)
    {
        Streader_set_error(sr, "No name specifier followed by %c", NAME_SPEC_SEP);
        return NULL;
    }
    else if (name_length > KQT_TRIGGER_NAME_MAX)
    {
        Streader_set_error(sr, "Name specifier is too long");
        return NULL;
    }

    Event_type trigger_type = Event_names_get(names, type_str);
    if (!Event_is_trigger(trigger_type))
    {
        Streader_set_error(
                sr, "Invalid or unsupported event type: \"%s\"", type_str);
        return NULL;
    }

    const char* name_setter = Event_names_get_name_event(names, type_str);
    if (name_setter == NULL)
    {
        Streader_set_error(
                sr, "No corresponding name event for event type: \"%s\"", type_str);
        return NULL;
    }

    Event_type name_setter_type = Event_names_get(names, name_setter);
    rassert(Event_is_trigger(name_setter_type));

    // Create the trigger
    Trigger* trigger = new_Trigger(name_setter_type, pos);
    if (trigger == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for a trigger");
        return NULL;
    }

    // Create the event description
    char event_desc[128] = "";
    snprintf(event_desc, 128, "[\"%s\", \"'%s'\"]", name_setter, name_arg);
    event_desc[127] = '\0';

    trigger->desc = memory_calloc_items(char, (int64_t)(strlen(event_desc) + 1));
    if (trigger->desc == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for a trigger");
        del_Trigger(trigger);
        return NULL;
    }

    strcpy(trigger->desc, event_desc);

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


