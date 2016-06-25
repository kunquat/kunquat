

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/Bind.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <expr.h>
#include <memory.h>
#include <player/Event_cache.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <Value.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Bind
{
    AAtree* cblists;
};


typedef struct Constraint
{
    char event_name[EVENT_NAME_MAX + 1];
    char* expr;
    struct Constraint* next;
} Constraint;


static Constraint* new_Constraint(Streader* sr);


static bool Constraint_match(
        Constraint* constraint, Event_cache* cache, Env_state* estate, Random* rand);


static void del_Constraint(Constraint* constraint);


static Target_event* new_Target_event(Streader* sr, const Event_names* names);


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


static bool read_constraints(Streader* sr, Bind* map, Cblist_item* item);


static bool read_events(Streader* sr, Cblist_item* item, const Event_names* names);


static bool Bind_is_cyclic(const Bind* map);


typedef struct bedata
{
    Bind* map;
    const Event_names* names;
} bedata;

static bool read_bind_entry(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);
    ignore(index);

    bedata* bd = userdata;

    char event_name[EVENT_NAME_MAX + 1] = "";
    if (!Streader_readf(sr, "[%s,", EVENT_NAME_MAX + 1, event_name))
        return false;

    Cblist* cblist = AAtree_get_exact(bd->map->cblists, event_name);
    if (cblist == NULL)
    {
        cblist = new_Cblist(event_name);
        if (cblist == NULL || !AAtree_ins(bd->map->cblists, cblist))
        {
            del_Cblist(cblist);
            Streader_set_memory_error(
                    sr, "Could not allocate memory for bind");
            return false;
        }
    }

    Cblist_item* item = new_Cblist_item();
    if (item == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        return false;
    }
    Cblist_append(cblist, item);

    if (!(read_constraints(sr, bd->map, item) &&
                Streader_match_char(sr, ',') &&
                read_events(sr, item, bd->names))
       )
        return false;

    return Streader_match_char(sr, ']');
}

Bind* new_Bind(Streader* sr, const Event_names* names)
{
    assert(sr != NULL);
    assert(names != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Bind* map = memory_alloc_item(Bind);
    if (map == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        return NULL;
    }

    map->cblists = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)del_Cblist);
    if (map->cblists == NULL)
    {
        del_Bind(map);
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        return NULL;
    }

    if (!Streader_has_data(sr))
        return map;

    bedata* bd = &(bedata){ .map = map, .names = names, };

    if (!Streader_read_list(sr, read_bind_entry, bd))
    {
        del_Bind(map);
        return NULL;
    }

    if (Bind_is_cyclic(map))
    {
        Streader_set_error(sr, "Bind contains a cycle");
        del_Bind(map);
        return NULL;
    }

    return map;
}


Event_cache* Bind_create_cache(const Bind* map)
{
    assert(map != NULL);

    Event_cache* cache = new_Event_cache();
    if (cache == NULL)
        return NULL;

    AAiter* iter = AAiter_init(AAITER_AUTO, map->cblists);
    Cblist* cblist = AAiter_get_at_least(iter, "");
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
        cblist = AAiter_get_next(iter);
    }

    return cache;
}


Target_event* Bind_get_first(
        const Bind* map,
        Event_cache* cache,
        Env_state* estate,
        const char* event_name,
        const Value* value,
        Random* rand)
{
    assert(map != NULL);
    assert(cache != NULL);
    assert(event_name != NULL);
    assert(value != NULL);

    Event_cache_update(cache, event_name, value);

    Cblist* list = AAtree_get_exact(map->cblists, event_name);
    if (list == NULL)
        return NULL;

    Cblist_item* item = list->first;
    while (item != NULL)
    {
        //fprintf(stderr, "%d\n", __LINE__);
        Constraint* constraint = item->constraints;
        while (constraint != NULL)
        {
            if (!Constraint_match(constraint, cache, estate, rand))
                break;

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
        return;

    del_AAtree(map->cblists);
    memory_free(map);

    return;
}


static bool Bind_dfs(const Bind* map, char* name);


static bool Bind_is_cyclic(const Bind* map)
{
    assert(map != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, map->cblists);
    Cblist* cblist = AAiter_get_at_least(iter, "");
    while (cblist != NULL)
    {
        assert(cblist->source_state != SOURCE_STATE_REACHED);
        if (cblist->source_state == SOURCE_STATE_VISITED)
        {
            cblist = AAiter_get_next(iter);
            continue;
        }

        assert(cblist->source_state == SOURCE_STATE_NEW);
        if (Bind_dfs(map, cblist->event_name))
            return true;

        cblist = AAiter_get_next(iter);
    }

    return false;
}


static bool Bind_dfs(const Bind* map, char* name)
{
    assert(map != NULL);
    assert(name != NULL);

    Cblist* cblist = AAtree_get_exact(map->cblists, name);
    if (cblist == NULL || cblist->source_state == SOURCE_STATE_VISITED)
        return false;

    if (cblist->source_state == SOURCE_STATE_REACHED)
        return true;

    assert(cblist->source_state == SOURCE_STATE_NEW);
    cblist->source_state = SOURCE_STATE_REACHED;

    Cblist_item* item = cblist->first;
    while (item != NULL)
    {
        Target_event* event = item->first_event;
        while (event != NULL)
        {
            Streader* sr = Streader_init(
                    STREADER_AUTO, event->desc, strlen(event->desc));
            char next_name[EVENT_NAME_MAX + 1] = "";
            Streader_readf(sr, "[%s", EVENT_NAME_MAX, next_name);
            assert(!Streader_is_error_set(sr));

            if (Bind_dfs(map, next_name))
                return true;

            event = event->next;
        }

        item = item->next;
    }

    cblist->source_state = SOURCE_STATE_VISITED;

    return false;
}


static bool read_constraint(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);
    ignore(index);

    Cblist_item* item = userdata;

    Constraint* constraint = new_Constraint(sr);
    if (constraint == NULL)
        return false;

    constraint->next = item->constraints;
    item->constraints = constraint;

    return true;
}

static bool read_constraints(Streader* sr, Bind* map, Cblist_item* item)
{
    assert(sr != NULL);
    assert(map != NULL);
    assert(item != NULL);

    return Streader_read_list(sr, read_constraint, item);
}


typedef struct edata
{
    Cblist_item* item;
    const Event_names* names;
} edata;

static bool read_event(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);
    ignore(index);

    edata* ed = userdata;

    Target_event* event = new_Target_event(sr, ed->names);
    if (event == NULL)
        return false;

    if (ed->item->last_event == NULL)
    {
        assert(ed->item->first_event == NULL);
        ed->item->first_event = ed->item->last_event = event;
    }
    else
    {
        assert(ed->item->first_event != NULL);
        ed->item->last_event->next = event;
        ed->item->last_event = event;
    }

    return true;
}

static bool read_events(
        Streader* sr, Cblist_item* item, const Event_names* names)
{
    assert(sr != NULL);
    assert(item != NULL);
    assert(names != NULL);

    edata* ed = &(edata){ .item = item, .names = names, };

    return Streader_read_list(sr, read_event, ed);
}


static Cblist* new_Cblist(char* event_name)
{
    assert(event_name != NULL);

    Cblist* list = memory_alloc_item(Cblist);
    if (list == NULL)
        return NULL;

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
        return;

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
        return NULL;

    item->constraints = NULL;
    item->first_event = NULL;
    item->last_event = NULL;
    item->next = NULL;

    return item;
}


static void del_Cblist_item(Cblist_item* item)
{
    if (item == NULL)
        return;

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


static Constraint* new_Constraint(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Constraint* c = memory_alloc_item(Constraint);
    if (c == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        return NULL;
    }

    c->expr = NULL;
    c->next = NULL;

    if (!Streader_readf(sr, "[%s,", EVENT_NAME_MAX + 1, c->event_name))
    {
        del_Constraint(c);
        return NULL;
    }

    Streader_skip_whitespace(sr);
    const char* const expr = Streader_get_remaining_data(sr);
    if (!Streader_read_string(sr, 0, NULL))
    {
        del_Constraint(c);
        return NULL;
    }

    const char* const expr_end = Streader_get_remaining_data(sr);

    if (!Streader_match_char(sr, ']'))
    {
        del_Constraint(c);
        return NULL;
    }

    assert(expr_end != NULL);
    assert(expr_end > expr);
    int len = expr_end - expr;

    c->expr = memory_calloc_items(char, len + 1);
    if (c->expr == NULL)
    {
        del_Constraint(c);
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        return NULL;
    }

    strncpy(c->expr, expr, len);
    c->expr[len] = '\0';

    return c;
}


static bool Constraint_match(
        Constraint* constraint, Event_cache* cache, Env_state* estate, Random* rand)
{
    assert(constraint != NULL);
    assert(cache != NULL);
    assert(estate != NULL);
    assert(rand != NULL);

    const Value* value = Event_cache_get_value(cache, constraint->event_name);
    assert(value != NULL);

    Value* result = VALUE_AUTO;
    Streader* sr = Streader_init(
            STREADER_AUTO, constraint->expr, strlen(constraint->expr));
    //fprintf(stderr, "%s, %s", constraint->event_name, constraint->expr);
    evaluate_expr(sr, estate, value, result, rand);
    //fprintf(stderr, ", %s", state->message);
    //fprintf(stderr, " -> %d %s\n", (int)result->type,
    //                               result->value.bool_type ? "true" : "false");

    return (result->type == VALUE_TYPE_BOOL) && result->value.bool_type;
}


static void del_Constraint(Constraint* constraint)
{
    if (constraint == NULL)
        return;

    memory_free(constraint->expr);
    memory_free(constraint);

    return;
}


static Target_event* new_Target_event(Streader* sr, const Event_names* names)
{
    assert(sr != NULL);
    assert(names != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Target_event* event = memory_alloc_item(Target_event);
    if (event == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        return NULL;
    }

    event->ch_offset = 0;
    event->desc = NULL;
    event->next = NULL;

    int64_t ch_offset = 0;
    if (!Streader_readf(sr, "[%i,", &ch_offset))
    {
        del_Target_event(event);
        return NULL;
    }

    if (ch_offset <= -KQT_COLUMNS_MAX || ch_offset >= KQT_COLUMNS_MAX)
    {
        Streader_set_error(sr, "Channel offset out of bounds");
        del_Target_event(event);
        return NULL;
    }

    Streader_skip_whitespace(sr);
    const char* const desc = Streader_get_remaining_data(sr);

    char event_name[EVENT_NAME_MAX + 1] = "";
    if (!Streader_readf(sr, "[%s,", EVENT_NAME_MAX + 1, event_name))
    {
        del_Target_event(event);
        return NULL;
    }

    if (Event_names_get(names, event_name) == Event_NONE)
    {
        Streader_set_error(sr, "Unsupported event type: %s", event_name);
        del_Target_event(event);
        return NULL;
    }

    Value_type type = Event_names_get_param_type(names, event_name);
    if (type == VALUE_TYPE_NONE)
        Streader_read_null(sr);
    else
        Streader_read_string(sr, 0, NULL);

    if (!Streader_readf(sr, "]]"))
    {
        del_Target_event(event);
        return NULL;
    }

    const char* const desc_end = Streader_get_remaining_data(sr);
    if (desc_end == NULL)
    {
        Streader_set_error(sr, "Unexpected end of data");
        del_Target_event(event);
        return NULL;
    }

    int len = desc_end - desc;
    assert(len > 0);
    event->desc = memory_alloc_items(char, len + 1);
    if (event->desc == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for bind");
        del_Target_event(event);
        return NULL;
    }

    event->ch_offset = ch_offset;
    memcpy(event->desc, desc, len);
    event->desc[len] = '\0';

    return event;
}


static void del_Target_event(Target_event* event)
{
    if (event == NULL)
        return;

    memory_free(event->desc);
    memory_free(event);

    return;
}


