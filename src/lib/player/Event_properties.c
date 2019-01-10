

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

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>
#include <player/Event_type.h>
#include <player/Param_validator.h>
#include <Value.h>

#include <stdlib.h>


typedef struct Entry
{
    Event_type type;
    Value_type param_type;
    Param_validator* validator;
    char name_setter[KQT_EVENT_NAME_MAX + 1];
} Entry;


#define ENTRY_KEY(event_type) \
    (&(Entry){ .type = (event_type), VALUE_TYPE_NONE, NULL, "" })


static void del_Entry(Entry* entry)
{
    ignore(entry);
    return;
}


static int Entry_cmp(const Entry* e1, const Entry* e2)
{
    rassert(e1 != NULL);
    rassert(e2 != NULL);

    if (e1->type < e2->type)
        return -1;
    else if (e1->type > e2->type)
        return 1;

    return 0;
}


static Entry entries[] =
{
#define EVENT_TYPE_DEF(name, category, type_suffix, arg_type, validator) \
    { Event_##category##_##type_suffix, VALUE_TYPE_##arg_type, validator, "" },
#define EVENT_TYPE_NS_DEF(name, category, type_suffix, arg_type, validator, ns) \
    { Event_##category##_##type_suffix, VALUE_TYPE_##arg_type, validator, ns },
#include <player/Event_types.h>

    { Event_NONE, VALUE_TYPE_NONE, NULL, "" }
};


struct Event_properties
{
    AAtree* entries;
};


Event_properties* new_Event_properties(void)
{
    Event_properties* props = memory_alloc_item(Event_properties);
    if (props == NULL)
        return NULL;

    props->entries = new_AAtree(
            (AAtree_item_cmp*)Entry_cmp, (AAtree_item_destroy*)del_Entry);
    if (props->entries == NULL)
    {
        del_Event_properties(props);
        return NULL;
    }

    for (int i = 0; entries[i].type != Event_NONE; ++i)
    {
        rassert(!AAtree_contains(props->entries, &entries[i]));

        if (!AAtree_ins(props->entries, &entries[i]))
        {
            del_Event_properties(props);
            return NULL;
        }
    }

    return props;
}


Value_type Event_properties_get_param_type(
        const Event_properties* props, Event_type event_type)
{
    rassert(props != NULL);

    Entry* entry = AAtree_get_exact(props->entries, ENTRY_KEY(event_type));
    rassert(entry != NULL);

    return entry->param_type;
}


Param_validator* Event_properties_get_param_validator(
        const Event_properties* props, Event_type event_type)
{
    rassert(props != NULL);

    Entry* entry = AAtree_get_exact(props->entries, ENTRY_KEY(event_type));
    rassert(entry != NULL);

    return entry->validator;
}


const char* Event_properties_get_name_event(
        const Event_properties* props, Event_type event_type)
{
    rassert(props != NULL);

    Entry* entry = AAtree_get_exact(props->entries, ENTRY_KEY(event_type));
    rassert(entry != NULL);

    return entry->name_setter;
}


void del_Event_properties(Event_properties* props)
{
    if (props == NULL)
        return;

    del_AAtree(props->entries);
    memory_free(props);

    return;
}


