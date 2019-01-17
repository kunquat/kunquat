

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Event_properties.h>

#include <debug/assert.h>
#include <player/Event_type.h>
#include <player/Param_validator.h>
#include <Value.h>

#include <stdlib.h>


typedef struct Entry
{
    Value_type param_type;
    Param_validator* validator;
    char name_setter[KQT_EVENT_NAME_MAX + 1];
} Entry;


static Entry entries[Event_STOP] =
{
#define EVENT_TYPE_DEF(name, category, type_suffix, arg_type, validator) \
    [Event_##category##_##type_suffix] = { VALUE_TYPE_##arg_type, validator, "" },
#define EVENT_TYPE_NS_DEF(name, category, type_suffix, arg_type, validator, ns) \
    [Event_##category##_##type_suffix] = { VALUE_TYPE_##arg_type, validator, ns },
#include <player/Event_types.h>
};


Value_type Event_properties_get_param_type(Event_type event_type)
{
    rassert(Event_is_valid(event_type));
    return entries[event_type].param_type;
}


Param_validator* Event_properties_get_param_validator(Event_type event_type)
{
    rassert(Event_is_valid(event_type));
    return entries[event_type].validator;
}


const char* Event_properties_get_name_event(Event_type event_type)
{
    rassert(Event_is_valid(event_type));
    return entries[event_type].name_setter;
}


