

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
#include <Event_type.h>
#include <File_base.h>
#include <xassert.h>
#include <xmemory.h>


struct Call_map
{
    AAtree* cache;
    AAtree* cblists;
};


typedef struct Constraint
{
    char event_name[EVENT_NAME_MAX + 1];
    char* expr;
    struct Constraint* next;
} Constraint;


static Constraint* new_Constraint(char** str, Read_state* state);


static void del_Constraint(Constraint* constraint);


typedef struct Target_event
{
    char* desc;
    struct Target_event* next;
} Target_event;


static Target_event* new_Target_event(char** str,
                                      Read_state* state,
                                      Event_names* names);


static void del_Target_event(Target_event* event);


typedef struct Cblist_item
{
    Constraint* constraints;
    Target_event* events;
    struct Cblist_item* next;
} Cblist_item;


static Cblist_item* new_Cblist_item(void);


static void del_Cblist_item(Cblist_item* item);


typedef struct Cblist
{
    char event_name[EVENT_NAME_MAX + 1];
    Cblist_item* first;
    Cblist_item* last;
} Cblist;


static Cblist* new_Cblist(char* event_name);


static void Cblist_append(Cblist* list, Cblist_item* item);


static void del_Cblist(Cblist* list);


static bool read_constraints(char** str,
                             Read_state* state,
                             Call_map* map,
                             Cblist_item* item);


static bool read_events(char** str,
                        Read_state* state,
                        Cblist_item* item,
                        Event_names* names);


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
        Cblist_item* item = new_Cblist_item();
        if (item == NULL)
        {
            del_Call_map(map);
            return NULL;
        }
        Cblist_append(cblist, item);
        str = read_const_char(str, ',', state);
        if (!read_constraints(&str, state, map, item))
        {
            del_Call_map(map);
            return NULL;
        }
        str = read_const_char(str, ',', state);
        if (!read_events(&str, state, item, names))
        {
            del_Call_map(map);
            return NULL;
        }
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


static bool read_constraints(char** str,
                             Read_state* state,
                             Call_map* map,
                             Cblist_item* item)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    assert(map != NULL);
    assert(item != NULL);
    *str = read_const_char(*str, '[', state);
    if (state->error)
    {
        return false;
    }
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        Read_state_clear_error(state);
        bool expect_entry = true;
        while (expect_entry)
        {
            Constraint* constraint = new_Constraint(str, state);
            if (constraint == NULL)
            {
                return false;
            }
            constraint->next = item->constraints;
            item->constraints = constraint;
            check_next(*str, state, expect_entry);
        }
    }
    return !state->error;
}


static bool read_events(char** str,
                        Read_state* state,
                        Cblist_item* item,
                        Event_names* names)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    assert(item != NULL);
    assert(names != NULL);
    *str = read_const_char(*str, '[', state);
    if (state->error)
    {
        return false;
    }
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        Read_state_clear_error(state);
        bool expect_entry = true;
        while (expect_entry)
        {
            Target_event* event = new_Target_event(str, state, names);
            if (event == NULL)
            {
                return false;
            }
            event->next = item->events;
            item->events = event;
            check_next(*str, state, expect_entry);
        }
    }
    return !state->error;
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


static void Cblist_append(Cblist* list, Cblist_item* item)
{
    assert(list != NULL);
    assert(item != NULL);
    if (list->last == NULL)
    {
        assert(list->first == NULL);
        list->first = list->last = item;
        return;
    }
    list->last->next = item;
    list->last = item;
    return;
}


static void del_Cblist(Cblist* list)
{
    if (list == NULL)
    {
        return;
    }
    Cblist_item* cur = list->first;
    while (cur != NULL)
    {
        Cblist_item* next = cur->next;
        del_Cblist_item(cur);
        cur = next;
    }
    xfree(list);
    return;
}


static Cblist_item* new_Cblist_item(void)
{
    Cblist_item* item = xalloc(Cblist_item);
    if (item == NULL)
    {
        return NULL;
    }
    item->constraints = NULL;
    item->events = NULL;
    item->next = NULL;
    return item;
}


static void del_Cblist_item(Cblist_item* item)
{
    if (item == NULL)
    {
        return;
    }
    Constraint* curc = item->constraints;
    while (curc != NULL)
    {
        Constraint* nextc = curc->next;
        del_Constraint(curc);
        curc = nextc;
    }
    Target_event* curt = item->events;
    while (curt != NULL)
    {
        Target_event* nextt = curt->next;
        del_Target_event(curt);
        curt = nextt;
    }
    xfree(item);
    return;
}


static Constraint* new_Constraint(char** str, Read_state* state)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Constraint* c = xalloc(Constraint);
    if (c == NULL)
    {
        return NULL;
    }
    c->expr = NULL;
    c->next = NULL;
    *str = read_const_char(*str, '[', state);
    *str = read_string(*str, c->event_name, EVENT_NAME_MAX + 1, state);
    *str = read_const_char(*str, ',', state);
    char expr[1024] = "";
    *str = read_string(*str, expr, 1024, state);
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Constraint(c);
        return NULL;
    }
    c->expr = xcalloc(char, strlen(expr) + 1);
    if (c->expr == NULL)
    {
        del_Constraint(c);
        return NULL;
    }
    strcpy(c->expr, expr);
    return c;
}


static void del_Constraint(Constraint* constraint)
{
    if (constraint == NULL)
    {
        return;
    }
    xfree(constraint->expr);
    xfree(constraint);
    return;
}


static Target_event* new_Target_event(char** str,
                                      Read_state* state,
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
    Target_event* event = xalloc(Target_event);
    if (event == NULL)
    {
        return NULL;
    }
    event->desc = NULL;
    event->next = NULL;
    char* desc = read_const_char(*str, '[', state);
    char event_name[EVENT_NAME_MAX + 1] = "";
    desc = read_string(desc, event_name, EVENT_NAME_MAX + 1, state);
    desc = read_const_char(desc, ',', state);
    desc = read_const_char(desc, '[', state);
    if (state->error)
    {
        del_Target_event(event);
        return NULL;
    }
    if (Event_names_get(names, event_name) == EVENT_NONE)
    {
        Read_state_set_error(state, "Unsupported event type: %s", event_name);
        del_Target_event(event);
        return NULL;
    }
    switch (Event_names_get_param_type(names, event_name))
    {
        case EVENT_FIELD_NONE:
        {
        } break;
        case EVENT_FIELD_BOOL:
        {
            desc = read_bool(desc, NULL, state);
        } break;
        case EVENT_FIELD_INT:
        {
            desc = read_int(desc, NULL, state);
        } break;
        case EVENT_FIELD_DOUBLE:
        {
            desc = read_double(desc, NULL, state);
        } break;
        case EVENT_FIELD_REAL:
        {
            desc = read_tuning(desc, NULL, NULL, state);
        } break;
        case EVENT_FIELD_RELTIME:
        {
            desc = read_reltime(desc, NULL, state);
        } break;
        case EVENT_FIELD_STRING:
        {
            desc = read_string(desc, NULL, 0, state);
        } break;
        default:
            assert(false);
    }
    desc = read_const_char(desc, ']', state);
    desc = read_const_char(desc, ']', state);
    if (state->error)
    {
        del_Target_event(event);
        return NULL;
    }
    int len = desc - *str;
    assert(len > 0);
    event->desc = xnalloc(char, len + 1);
    if (event->desc == NULL)
    {
        del_Target_event(event);
        return NULL;
    }
    memcpy(event->desc, *str, len);
    event->desc[len] = '\0';
    *str = desc;
    return event;
}


static void del_Target_event(Target_event* event)
{
    if (event == NULL)
    {
        return;
    }
    xfree(event->desc);
    xfree(event);
    return;
}


