

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
#include <Call_map.h>
#include <Event_names.h>
#include <File_base.h>
#include <xassert.h>
#include <xmemory.h>


struct Call_map
{
    AAtree* cache;
    AAtree* cblists;
};


typedef struct Cblist
{
    char event_name[EVENT_NAME_MAX + 1];
} Cblist;


static Cblist* new_Cblist(char* event_name);


static void del_Cblist(Cblist* list);


Call_map* new_Call_map(char* str,
                       Event_names* names,
                       Read_state* state)
{
    assert(names != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Call_map* map = xalloc(Call_map);
    if (map == NULL)
    {
        return NULL;
    }
    map->cache = NULL;
    map->cblists = new_AAtree((int (*)(const void*, const void*))strcmp,
                              (void (*)(void*))del_Cblist);
    if (map->cblists == NULL)
    {
        del_Call_map(map);
        return NULL;
    }
    if (str == NULL)
    {
        return map;
    }
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Call_map(map);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        del_Call_map(map);
        return NULL;
    }
    Read_state_clear_error(state);
    bool expect_entry = true;
    while (expect_entry)
    {
        str = read_const_char(str, '[', state);
        char event_name[EVENT_NAME_MAX + 1];
        str = read_string(str, event_name, EVENT_NAME_MAX + 1, state);
        if (state->error)
        {
            del_Call_map(map);
            return NULL;
        }
        Cblist* cblist = AAtree_get_exact(map->cblists, event_name);
        if (cblist == NULL)
        {
            cblist = new_Cblist(event_name);
            if (cblist == NULL)
            {
                del_Call_map(map);
                return NULL;
            }
            if (!AAtree_ins(map->cblists, cblist))
            {
                del_Cblist(cblist);
                del_Call_map(map);
                return NULL;
            }
        }
        str = read_const_char(str, ',', state);
        // TODO: read constraint list
        str = read_const_char(str, ',', state);
        // TODO: read events to be fired
        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_Call_map(map);
            return NULL;
        }
        check_next(str, state, expect_entry);
    }
    return map;
}


bool Call_map_get_first(Call_map* map,
                        char* src_event,
                        char* dest_event,
                        int dest_size)
{
    assert(map != NULL);
    assert(src_event != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    // TODO
    return false;
}


bool Call_map_get_next(Call_map* map,
                       char* dest_event,
                       int dest_size)
{
    assert(map != NULL);
    assert(dest_event != NULL);
    assert(dest_size > 0);
    // TODO
    return false;
}


void del_Call_map(Call_map* map)
{
    if (map == NULL)
    {
        return;
    }
    del_AAtree(map->cache);
    del_AAtree(map->cblists);
    xfree(map);
    return;
}


static Cblist* new_Cblist(char* event_name)
{
    assert(event_name != NULL);
    Cblist* list = xalloc(Cblist);
    if (list == NULL)
    {
        return NULL;
    }
    strncpy(list->event_name, event_name, EVENT_NAME_MAX);
    list->event_name[EVENT_NAME_MAX] = '\0';
    return list;
}


static void del_Cblist(Cblist* list)
{
    if (list == NULL)
    {
        return;
    }
    xfree(list);
    return;
}


