

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
#include <stdbool.h>
#include <string.h>

#include <AAtree.h>
#include <Event.h>
#include <Event_names.h>
#include <Event_type.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct Name_info
{
    char name[EVENT_NAME_MAX];
    Event_type type;
} Name_info;


struct Event_names
{
    AAtree* names;
    bool error;
};


Event_names* new_Event_names(void)
{
    Event_names* names = xalloc(Event_names);
    if (names == NULL)
    {
        return NULL;
    }
    names->error = false;
    names->names = new_AAtree((int (*)(const void*, const void*))strcmp,
                              free);
    if (names->names == NULL)
    {
        del_Event_names(names);
        return NULL;
    }
    return names;
}


bool Event_names_add(Event_names* names, const char* name, Event_type type)
{
    assert(names != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(Event_type_is_supported(type));
    if (names->error)
    {
        return false;
    }
    Name_info* info = xalloc(Name_info);
    if (info == NULL || !AAtree_ins(names->names, info))
    {
        xfree(info);
        names->error = true;
        return false;
    }
    return true;
}


bool Event_names_error(Event_names* names)
{
    assert(names != NULL);
    return names->error;
}


Event_type Event_names_get(Event_names* names, const char* name)
{
    assert(names != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    Name_info* info = AAtree_get(names->names, name);
    if (info == NULL)
    {
        return EVENT_NONE;
    }
    return info->type;
}


void del_Event_names(Event_names* names)
{
    if (names == NULL)
    {
        return;
    }
    del_AAtree(names->names);
    xfree(names);
    return;
}


