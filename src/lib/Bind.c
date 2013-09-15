

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
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
#include <stdio.h>

#include <AAtree.h>
#include <Bind.h>
#include <Event_cache.h>
#include <Event_names.h>
#include <Event_type.h>
#include <expr.h>
#include <File_base.h>
#include <memory.h>
#include <Random.h>
#include <Value.h>
#include <xassert.h>


struct Bind
{
    AAiter* iter;
    AAtree* cblists;
};


typedef struct Constraint
{
    char event_name[EVENT_NAME_MAX + 1];
    char* expr;
    struct Constraint* next;
} Constraint;


static Constraint* new_Constraint(char** str, Read_state* state);


static bool Constraint_match(Constraint* constraint, Event_cache* cache,
                             Env_state* estate, Random* rand);


static void del_Constraint(Constraint* constraint);


static Target_event* new_Target_event(char** str,
                                      Read_state* state,
                                      Event_names* names);


static void del_Target_event(Target_event* event);


typedef struct Cblist_item
{
    Constraint* constraints;
    Target_event* first_event;
    Target_event* last_event;
    struct Cblist_item* next;
} Cblist_item;


static Cblist_item* new_Cblist_item(void);


static void del_Cblist_item(Cblist_item* item);


typedef enum
{
    SOURCE_STATE_NEW = 0,
    SOURCE_STATE_REACHED,
    SOURCE_STATE_VISITED
} Source_state;


typedef struct Cblist
{
    char event_name[EVENT_NAME_MAX + 1];
    Source_state source_state;
    Cblist_item* first;
    Cblist_item* last;
} Cblist;


static Cblist* new_Cblist(char* event_name);


static void Cblist_append(Cblist* list, Cblist_item* item);


static void del_Cblist(Cblist* list);


static bool read_constraints(char** str,
                             Read_state* state,
                             Bind* map,
                             Cblist_item* item);


static bool read_events(char** str,
                        Read_state* state,
                        Cblist_item* item,
                        Event_names* names);


static bool Bind_is_cyclic(Bind* map);


Bind* new_Bind(char* str, Event_names* names, Read_state* state)
{
    assert(names != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Bind* map = memory_alloc_item(Bind);
    if (map == NULL)
    {
        return NULL;
    }
    map->iter = new_AAiter(NULL);
    map->cblists = new_AAtree((int (*)(const void*, const void*))strcmp,
                              (void (*)(void*))del_Cblist);
    if (map->iter == NULL || map->cblists == NULL)
    {
        del_Bind(map);
        return NULL;
    }
    if (str == NULL)
    {
        return map;
    }
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Bind(map);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        return map;
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
            del_Bind(map);
            return NULL;
        }
        Cblist* cblist = AAtree_get_exact(map->cblists, event_name);
        if (cblist == NULL)
        {
            cblist = new_Cblist(event_name);
            if (cblist == NULL)
            {
                del_Bind(map);
                return NULL;
            }
            if (!AAtree_ins(map->cblists, cblist))
            {
                del_Cblist(cblist);
                del_Bind(map);
                return NULL;
            }
        }
        Cblist_item* item = new_Cblist_item();
        if (item == NULL)
        {
            del_Bind(map);
            return NULL;
        }
        Cblist_append(cblist, item);
        str = read_const_char(str, ',', state);
        if (!read_constraints(&str, state, map, item))
        {
            del_Bind(map);
            return NULL;
        }
        str = read_const_char(str, ',', state);
        if (!read_events(&str, state, item, names))
        {
            del_Bind(map);
            return NULL;
        }
        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_Bind(map);
            return NULL;
        }
        check_next(str, state, expect_entry);
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        del_Bind(map);
        return NULL;
    }
    if (Bind_is_cyclic(map))
    {
        Read_state_set_error(state, "Bind contains a cycle");
        del_Bind(map);
        return NULL;
    }
    return map;
}


Event_cache* Bind_create_cache(Bind* map)
{
    assert(map != NULL);
    Event_cache* cache = new_Event_cache();
    if (cache == NULL)
    {
        return NULL;
    }
    AAiter_change_tree(map->iter, map->cblists);
    Cblist* cblist = AAiter_get_at_least(map->iter, "");
    while (cblist != NULL)
    {
        Cblist_item* item = cblist->first;
        while (item != NULL)
        {
            Constraint* constraint = item->constraints;
            while (constraint != NULL)
            {
                if (!Event_cache_add_event(cache, constraint->event_name))
                {
                    del_Event_cache(cache);
                    return NULL;
                }
                constraint = constraint->next;
            }
            item = item->next;
        }
        cblist = AAiter_get_next(map->iter);
    }
    return cache;
}


Target_event* Bind_get_first(Bind* map,
                             Event_cache* cache,
                             Env_state* estate,
                             char* event_name,
                             Value* value,
                             Random* rand)
{
    assert(map != NULL);
    assert(cache != NULL);
    assert(event_name != NULL);
    assert(value != NULL);
    Event_cache_update(cache, event_name, value);
    Cblist* list = AAtree_get_exact(map->cblists, event_name);
    if (list == NULL)
    {
        return NULL;
    }
    Cblist_item* item = list->first;
    while (item != NULL)
    {
        //fprintf(stderr, "%d\n", __LINE__);
        Constraint* constraint = item->constraints;
        while (constraint != NULL)
        {
            if (!Constraint_match(constraint, cache, estate, rand))
            {
                break;
            }
            constraint = constraint->next;
        }
        //fprintf(stderr, "%d\n", __LINE__);
        if (constraint == NULL)
        {
            //fprintf(stderr, "item->events: %s\n", item->events->desc);
            return item->first_event;
        }
        //fprintf(stderr, "%d\n", __LINE__);
        item = item->next;
    }
    return NULL;
}


void del_Bind(Bind* map)
{
    if (map == NULL)
    {
        return;
    }
    del_AAiter(map->iter);
    del_AAtree(map->cblists);
    memory_free(map);
    return;
}


static bool Bind_dfs(Bind* map, char* name);


static bool Bind_is_cyclic(Bind* map)
{
    assert(map != NULL);
    AAiter_change_tree(map->iter, map->cblists);
    Cblist* cblist = AAiter_get_at_least(map->iter, "");
    while (cblist != NULL)
    {
        assert(cblist->source_state != SOURCE_STATE_REACHED);
        if (cblist->source_state == SOURCE_STATE_VISITED)
        {
            cblist = AAiter_get_next(map->iter);
            continue;
        }
        assert(cblist->source_state == SOURCE_STATE_NEW);
        if (Bind_dfs(map, cblist->event_name))
        {
            return true;
        }
        cblist = AAiter_get_next(map->iter);
    }
    return false;
}


static bool Bind_dfs(Bind* map, char* name)
{
    assert(map != NULL);
    assert(name != NULL);
    Cblist* cblist = AAtree_get_exact(map->cblists, name);
    if (cblist == NULL || cblist->source_state == SOURCE_STATE_VISITED)
    {
        return false;
    }
    if (cblist->source_state == SOURCE_STATE_REACHED)
    {
        return true;
    }
    assert(cblist->source_state == SOURCE_STATE_NEW);
    cblist->source_state = SOURCE_STATE_REACHED;
    Cblist_item* item = cblist->first;
    while (item != NULL)
    {
        Target_event* event = item->first_event;
        while (event != NULL)
        {
            Read_state* state = READ_STATE_AUTO;
            char next_name[EVENT_NAME_MAX + 1] = "";
            char* str = read_const_char(event->desc, '[', state);
            read_string(str, next_name, EVENT_NAME_MAX, state);
            assert(!state->error);
            if (Bind_dfs(map, next_name))
            {
                return true;
            }
            event = event->next;
        }
        item = item->next;
    }
    cblist->source_state = SOURCE_STATE_VISITED;
    return false;
}


static bool read_constraints(char** str,
                             Read_state* state,
                             Bind* map,
                             Cblist_item* item)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(state != NULL);
    assert(map != NULL);
    (void)map;
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
        *str = read_const_char(*str, ']', state);
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
            if (item->last_event == NULL)
            {
                assert(item->first_event == NULL);
                item->first_event = item->last_event = event;
            }
            else
            {
                assert(item->first_event != NULL);
                item->last_event->next = event;
                item->last_event = event;
            }
            check_next(*str, state, expect_entry);
        }
        *str = read_const_char(*str, ']', state);
    }
    return !state->error;
}


static Cblist* new_Cblist(char* event_name)
{
    assert(event_name != NULL);
    Cblist* list = memory_alloc_item(Cblist);
    if (list == NULL)
    {
        return NULL;
    }
    list->source_state = SOURCE_STATE_NEW;
    list->first = list->last = NULL;
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
    memory_free(list);
    return;
}


static Cblist_item* new_Cblist_item(void)
{
    Cblist_item* item = memory_alloc_item(Cblist_item);
    if (item == NULL)
    {
        return NULL;
    }
    item->constraints = NULL;
    item->first_event = NULL;
    item->last_event = NULL;
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
    Target_event* curt = item->first_event;
    while (curt != NULL)
    {
        Target_event* nextt = curt->next;
        del_Target_event(curt);
        curt = nextt;
    }
    memory_free(item);
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
    Constraint* c = memory_alloc_item(Constraint);
    if (c == NULL)
    {
        return NULL;
    }
    c->expr = NULL;
    c->next = NULL;
    *str = read_const_char(*str, '[', state);
    *str = read_string(*str, c->event_name, EVENT_NAME_MAX + 1, state);
    *str = read_const_char(*str, ',', state);
    char* expr = skip_whitespace(*str, state);
    *str = read_string(expr, NULL, 0, state);
    if (state->error)
    {
        del_Constraint(c);
        return NULL;
    }
    assert(expr < *str);
    int len = *str - expr;
    *str = read_const_char(*str, ']', state);
    if (state->error)
    {
        del_Constraint(c);
        return NULL;
    }
    c->expr = memory_calloc_items(char, len + 1);
    if (c->expr == NULL)
    {
        del_Constraint(c);
        return NULL;
    }
    strncpy(c->expr, expr, len);
    c->expr[len] = '\0';
    return c;
}


static bool Constraint_match(Constraint* constraint, Event_cache* cache,
                             Env_state* estate, Random* rand)
{
    assert(constraint != NULL);
    assert(cache != NULL);
    assert(estate != NULL);
    assert(rand != NULL);
    Value* value = Event_cache_get_value(cache, constraint->event_name);
    assert(value != NULL);
    Value* result = VALUE_AUTO;
    Read_state* state = READ_STATE_AUTO;
    //fprintf(stderr, "%s, %s", constraint->event_name, constraint->expr);
    evaluate_expr(constraint->expr, estate, state, value, result, rand);
    //fprintf(stderr, ", %s", state->message);
    //fprintf(stderr, " -> %d %s\n", (int)result->type,
    //                               result->value.bool_type ? "true" : "false");
    return result->type == VALUE_TYPE_BOOL && result->value.bool_type;
}


static void del_Constraint(Constraint* constraint)
{
    if (constraint == NULL)
    {
        return;
    }
    memory_free(constraint->expr);
    memory_free(constraint);
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
    Target_event* event = memory_alloc_item(Target_event);
    if (event == NULL)
    {
        return NULL;
    }
    event->ch_offset = 0;
    event->desc = NULL;
    event->next = NULL;
    *str = read_const_char(*str, '[', state);
    int64_t ch_offset = 0;
    *str = read_int(*str, &ch_offset, state);
    *str = read_const_char(*str, ',', state);
    *str = skip_whitespace(*str, state);
    char* desc = read_const_char(*str, '[', state);
    char event_name[EVENT_NAME_MAX + 1] = "";
    desc = read_string(desc, event_name, EVENT_NAME_MAX + 1, state);
    desc = read_const_char(desc, ',', state);
    if (state->error)
    {
        del_Target_event(event);
        return NULL;
    }
    if (ch_offset <= -KQT_COLUMNS_MAX || ch_offset >= KQT_COLUMNS_MAX)
    {
        Read_state_set_error(state, "Channel offset out of bounds");
        del_Target_event(event);
        return NULL;
    }
    if (Event_names_get(names, event_name) == Event_NONE)
    {
        Read_state_set_error(state, "Unsupported event type: %s", event_name);
        del_Target_event(event);
        return NULL;
    }
    Value_type type = Event_names_get_param_type(names, event_name);
    if (type == VALUE_TYPE_NONE)
    {
        desc = read_null(desc, state);
    }
    else
    {
        desc = read_string(desc, NULL, 0, state);
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
    event->desc = memory_alloc_items(char, len + 1);
    if (event->desc == NULL)
    {
        del_Target_event(event);
        return NULL;
    }
    event->ch_offset = ch_offset;
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
    memory_free(event->desc);
    memory_free(event);
    return;
}


