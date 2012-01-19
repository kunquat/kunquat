

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>

#include <AAtree.h>
#include <Event_cache.h>
#include <Event_names.h>
#include <Event_type.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


struct Event_cache
{
    AAiter* iter;
    AAtree* cache;
};


typedef struct Event_state
{
    char event_name[EVENT_NAME_MAX + 1];
    Value value;
} Event_state;


static Event_state* new_Event_state(char* event_name);


static void Event_state_reset(Event_state* es);


static void del_Event_state(Event_state* es);


Event_cache* new_Event_cache(void)
{
    Event_cache* cache = xalloc(Event_cache);
    if (cache == NULL)
    {
        return NULL;
    }
    cache->iter = new_AAiter(NULL);
    cache->cache = new_AAtree((int (*)(const void*, const void*))strcmp,
                              (void (*)(void*))del_Event_state);
    if (cache->iter == NULL || cache->cache == NULL)
    {
        del_Event_cache(cache);
        return NULL;
    }
    return cache;
}


bool Event_cache_add_event(Event_cache* cache, char* event_name)
{
    assert(cache != NULL);
    assert(event_name != NULL);
    if (AAtree_get_exact(cache->cache, event_name) != NULL)
    {
        return true;
    }
    Event_state* es = new_Event_state(event_name);
    if (es == NULL || !AAtree_ins(cache->cache, es))
    {
        return false;
    }
    return true;
}


void Event_cache_reset(Event_cache* cache)
{
    assert(cache != NULL);
    AAiter_change_tree(cache->iter, cache->cache);
    Event_state* es = AAiter_get(cache->iter, "");
    while (es != NULL)
    {
        Event_state_reset(es);
        es = AAiter_get_next(cache->iter);
    }
    return;
}


void del_Event_cache(Event_cache* cache)
{
    if (cache == NULL)
    {
        return;
    }
    del_AAiter(cache->iter);
    del_AAtree(cache->cache);
    xfree(cache);
    return;
}


static Event_state* new_Event_state(char* event_name)
{
    assert(event_name != NULL);
    assert(strlen(event_name) <= EVENT_NAME_MAX);
    Event_state* es = xalloc(Event_state);
    if (es == NULL)
    {
        return NULL;
    }
    strcpy(es->event_name, event_name);
    es->value.type = VALUE_TYPE_NONE;
    return es;
}


static void Event_state_reset(Event_state* es)
{
    assert(es != NULL);
    es->value.type = VALUE_TYPE_NONE;
    return;
}


static void del_Event_state(Event_state* es)
{
    if (es == NULL)
    {
        return;
    }
    xfree(es);
    return;
}


