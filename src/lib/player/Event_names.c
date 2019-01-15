

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Event_names.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/Event_properties.h>
#include <player/Event_type.h>
#include <player/Param_validator.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


typedef struct Name_info
{
    char name[KQT_EVENT_NAME_MAX + 1];
    Event_type type;
    /*
    Value_type param_type;
    Param_validator* validator;
    char name_setter[KQT_EVENT_NAME_MAX + 1];
    */
} Name_info;


static void del_Name_info(Name_info* ni);


static void del_Name_info(Name_info* ni)
{
    ignore(ni);
    return;
}


static Name_info event_specs[] =
{
#define EVENT_TYPE_DEF(name, category, type_suffix, arg_type, validator) \
    { name, Event_##category##_##type_suffix },
#define EVENT_TYPE_NS_DEF(name, category, type_suffix, arg_type, validator, ns) \
    { name, Event_##category##_##type_suffix },
#include <player/Event_types.h>

    { "", Event_NONE }
};


struct Event_names
{
    AAtree* names;
};


static int event_name_cmp(const char* e1, const char* e2);


Event_names* new_Event_names(void)
{
    Event_names* names = memory_alloc_item(Event_names);
    if (names == NULL)
        return NULL;

    names->names = new_AAtree(
            (AAtree_item_cmp*)event_name_cmp, (AAtree_item_destroy*)del_Name_info);
    if (names->names == NULL)
    {
        del_Event_names(names);
        return NULL;
    }

    for (int i = 0; event_specs[i].name[0] != '\0'; ++i)
    {
        rassert(strlen(event_specs[i].name) > 0);
        rassert(strlen(event_specs[i].name) < KQT_EVENT_NAME_MAX);
        rassert(!AAtree_contains(names->names, &event_specs[i]));

        if (!AAtree_ins(names->names, &event_specs[i]))
        {
            del_Event_names(names);
            return NULL;
        }
    }

    return names;
}


static int event_name_cmp(const char* e1, const char* e2)
{
    rassert(e1 != NULL);
    rassert(e2 != NULL);

    int i = 0;
    for (i = 0; e1[i] != '\0' && e1[i] != '"' &&
                e2[i] != '\0' && e1[i] != '"'; ++i)
    {
        if (e1[i] < e2[i])
            return -1;
        else if (e1[i] > e2[i])
            return 1;
    }

    if (e2[i] != '\0' && e2[i] != '"')
        return -1;
    if (e1[i] != '\0' && e1[i] != '"')
        return 1;

    return 0;
}


Event_type Event_names_get(const Event_names* names, const char* name)
{
    rassert(names != NULL);
    rassert(name != NULL);

    const Name_info* info = AAtree_get_exact(names->names, name);
    if (info == NULL)
        return Event_NONE;

    return info->type;
}


Value_type Event_names_get_param_type(const Event_names* names, const char* name)
{
    rassert(names != NULL);
    rassert(name != NULL);

    return Event_properties_get_param_type(Event_names_get(names, name));
}


Param_validator* Event_names_get_param_validator(
        const Event_names* names, const char* name)
{
    rassert(names != NULL);
    rassert(name != NULL);

    return Event_properties_get_param_validator(Event_names_get(names, name));
}


const char* Event_names_get_name_event(const Event_names* names, const char* name)
{
    rassert(names != NULL);
    rassert(name != NULL);

    const char* name_setter =
        Event_properties_get_name_event(Event_names_get(names, name));

    return (name_setter[0] != '\0') ? name_setter : NULL;
}


void del_Event_names(Event_names* names)
{
    if (names == NULL)
        return;

    del_AAtree(names->names);
    memory_free(names);

    return;
}


