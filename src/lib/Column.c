

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <Column.h>
#include <Event_global_jump.h>
#include <Event_names.h>
#include <memory.h>
#include <Tstamp.h>
#include <xassert.h>


struct Column
{
    Tstamp len;
    uint32_t version;
    Column_iter* edit_iter;
    AAtree* events;
};


static Event_list* new_Event_list(Event_list* nil, Event* event);

static Event_list* Event_list_init(Event_list* elist);

static int Event_list_cmp(const Event_list* list1, const Event_list* list2);

static void del_Event_list(Event_list* elist);


static Event_list* new_Event_list(Event_list* nil, Event* event)
{
    assert(!(nil == NULL) || (event == NULL));
    assert(!(event == NULL) || (nil == NULL));

    Event_list* elist = memory_alloc_item(Event_list);
    if (elist == NULL)
    {
        return NULL;
    }
    if (nil == NULL)
    {
        assert(event == NULL);
        elist->event = NULL;
        elist->prev = elist->next = elist;
    }
    else
    {
        assert(event != NULL);
        elist->event = event;
        elist->prev = elist->next = nil;
    }

    return elist;
}


static Event_list* Event_list_init(Event_list* elist)
{
    assert(elist != NULL);
    elist->prev = elist->next = elist;
    return elist;
}


static int Event_list_cmp(const Event_list* list1, const Event_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);
    Event* ev1 = list1->next->event;
    Event* ev2 = list2->next->event;
    assert(ev1 != NULL);
    assert(ev2 != NULL);
    return Tstamp_cmp(Event_get_pos(ev1), Event_get_pos(ev2));
}


Column_iter* new_Column_iter(Column* col)
{
    Column_iter* iter = memory_alloc_item(Column_iter);
    if (iter == NULL)
    {
        return NULL;
    }

    Column_iter_init(iter);

    if (col != NULL)
    {
        iter->version = col->version;
        iter->col = col;
        iter->tree_iter.tree = col->events;
    }

    return iter;
}


void Column_iter_init(Column_iter* iter)
{
    assert(iter != NULL);

    iter->version = 0;
    iter->col = NULL;
    iter->tree_iter = *AAITER_AUTO;
    iter->elist = NULL;

    return;
}


void Column_iter_change_col(Column_iter* iter, Column* col)
{
    assert(iter != NULL);
    assert(col != NULL);
    iter->col = col;
    iter->version = col->version;
    AAiter_change_tree(&iter->tree_iter, col->events);
    iter->elist = NULL;
    return;
}


Event* Column_iter_get(Column_iter* iter, const Tstamp* pos)
{
    assert(iter != NULL);
    assert(pos != NULL);

    iter->elist = Column_iter_get_row(iter, pos);
    if (iter->elist == NULL)
    {
        return NULL;
    }

    assert(iter->elist->event == NULL);
    assert(iter->elist->next != iter->elist);
    iter->elist = iter->elist->next;
    assert(iter->elist->event != NULL);
    return iter->elist->event;
}


Event_list* Column_iter_get_row(Column_iter* iter, const Tstamp* pos)
{
    assert(iter != NULL);
    assert(pos != NULL);

    if (iter->col == NULL)
        return NULL;

    iter->version = iter->col->version;
    Event* event = &(Event){ .type = Event_NONE };
    Tstamp_copy(&event->pos, pos);
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    iter->elist = AAiter_get_at_least(&iter->tree_iter, key);

    return iter->elist;
}


Event* Column_iter_get_next(Column_iter* iter)
{
    assert(iter != NULL);
    if (iter->elist == NULL || iter->col == NULL)
    {
        return NULL;
    }
    if (iter->version != iter->col->version)
    {
        iter->elist = NULL;
        iter->version = iter->col->version;
        return NULL;
    }
    assert(iter->elist->event != NULL);
    assert(iter->elist != iter->elist->next);
    iter->elist = iter->elist->next;
    if (iter->elist->event != NULL)
    {
        return iter->elist->event;
    }
    iter->elist = Column_iter_get_next_row(iter);
    if (iter->elist == NULL)
    {
        return NULL;
    }
    assert(iter->elist->event == NULL);
    assert(iter->elist->next != iter->elist);
    iter->elist = iter->elist->next;
    assert(iter->elist->event != NULL);
    return iter->elist->event;
}


Event_list* Column_iter_get_next_row(Column_iter* iter)
{
    assert(iter != NULL);

    if (iter->elist == NULL || iter->col == NULL)
        return NULL;

    if (iter->version != iter->col->version)
    {
        iter->elist = NULL;
        iter->version = iter->col->version;
        return NULL;
    }

    iter->elist = AAiter_get_next(&iter->tree_iter);

    return iter->elist;
}


void del_Column_iter(Column_iter* iter)
{
    if (iter == NULL)
    {
        return;
    }
    memory_free(iter);
    return;
}


static bool Column_parse(Column* col,
                         char* str,
                         const Event_names* event_names,
                         Read_state* state);


Column* new_Column(const Tstamp* len)
{
    Column* col = memory_alloc_item(Column);
    if (col == NULL)
    {
        return NULL;
    }
    col->version = 1;
    col->events = new_AAtree((int (*)(const void*, const void*))Event_list_cmp,
            (void (*)(void*))del_Event_list);
    if (col->events == NULL)
    {
        memory_free(col);
        return NULL;
    }
    col->edit_iter = new_Column_iter(col);
    if (col->edit_iter == NULL)
    {
        del_AAtree(col->events);
        memory_free(col);
        return NULL;
    }
    if (len != NULL)
    {
        Tstamp_copy(&col->len, len);
    }
    else
    {
        Tstamp_set(&col->len, INT64_MAX, 0);
    }
    return col;
}


Column* new_Column_from_string(const Tstamp* len,
                               char* str,
                               AAtree* locations,
                               AAiter* locations_iter,
                               const Event_names* event_names,
                               Read_state* state)
{
    assert(event_names != NULL);
    assert(state != NULL);
    assert(locations != NULL);
    assert(locations_iter != NULL);
    if (state->error)
    {
        return false;
    }
    Column* col = new_Column(len);
    if (col == NULL)
    {
        return NULL;
    }
    if (!Column_parse(col, str, event_names, state))
    {
        del_Column(col);
        return NULL;
    }
    if (!Column_update_locations(col, locations, locations_iter))
    {
        del_Column(col);
        return NULL;
    }
    return col;
}


#define break_if(error)   \
    if (true)             \
    {                     \
        if ((error))      \
        {                 \
            return false; \
        }                 \
    } else (void)0

static bool Column_parse(Column* col,
                         char* str,
                         const Event_names* event_names,
                         Read_state* state)
{
    assert(col != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (str == NULL)
    {
        // An empty Column has no properties, so we're done.
        return true;
    }
    str = read_const_char(str, '[', state); // start of Column
    break_if(state->error);
    str = read_const_char(str, ']', state); // check of empty Column
    if (!state->error)
    {
        return true;
    }
    Read_state_clear_error(state);

    bool expect_event = true;
    while (expect_event)
    {
        Event* event = new_Event_from_string(&str, state, event_names);
        if (event == NULL)
        {
            return false;
        }
        if (!Column_ins(col, event))
        {
            del_Event(event);
            return false;
        }
        check_next(str, state, expect_event);
    }

    str = read_const_char(str, ']', state);
    if (state->error)
    {
        return false;
    }
    return true;
}

#undef break_if


bool Column_update_locations(Column* col,
                             AAtree* locations,
                             AAiter* locations_iter)
{
    assert(col != NULL);
    assert(locations != NULL);
    assert(locations_iter != NULL);
    Column_iter_change_col(col->edit_iter, col);
    Event* event = Column_iter_get(
            col->edit_iter,
            Tstamp_init(TSTAMP_AUTO));
    while (event != NULL)
    {
        Event_type type = Event_get_type(event);
        if (type == Trigger_jump &&
                !Trigger_global_jump_set_locations(
                    (Event_global_jump*)event,
                    locations,
                    locations_iter))
        {
            return false;
        }
        event = Column_iter_get_next(col->edit_iter);
    }
    return true;
}


bool Column_ins(Column* col, Event* event)
{
    assert(col != NULL);
    assert(event != NULL);
    ++col->version;
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* ret = AAtree_get_at_least(col->events, key);
    if (ret == NULL || Tstamp_cmp(Event_get_pos(event),
            Event_get_pos(ret->next->event)) != 0)
    {
        Event_list* nil = new_Event_list(NULL, NULL);
        if (nil == NULL)
        {
            return false;
        }
        Event_list* node = new_Event_list(nil, event);
        if (node == NULL)
        {
            del_Event_list(nil);
            return false;
        }
        nil->prev = nil->next = node;
        node->prev = node->next = nil;
        if (!AAtree_ins(col->events, nil))
        {
            del_Event_list(nil);
            del_Event_list(node);
            return false;
        }
        return true;
    }
    assert(ret->next != ret);
    assert(ret->prev != ret);
    assert(ret->event == NULL);
    Event_list* node = new_Event_list(ret, event);
    if (node == NULL)
    {
        return false;
    }
    node->prev = ret->prev;
    node->next = ret;
    ret->prev->next = node;
    ret->prev = node;
    return true;
}


void del_Column(Column* col)
{
    if (col == NULL)
    {
        return;
    }
    del_AAtree(col->events);
    del_Column_iter(col->edit_iter);
    memory_free(col);
    return;
}


static void del_Event_list(Event_list* elist)
{
    if (elist == NULL)
    {
        return;
    }
    assert(elist->event == NULL);
    Event_list* cur = elist->next;
    assert(cur->event != NULL);
    while (cur->event != NULL)
    {
        Event_list* next = cur->next;
        assert(cur->event != NULL);
        if (!cur->copy)
        {
            del_Event(cur->event);
        }
        memory_free(cur);
        cur = next;
    }
    assert(cur == elist);
    memory_free(cur);
    return;
}


