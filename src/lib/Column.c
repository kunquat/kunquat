

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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <Reltime.h>
#include <Event_pg.h>
#include <Event_global_set_tempo.h>
#include <Event_names.h>
#include <Column.h>
#include <xassert.h>
#include <xmemory.h>


#define COLUMN_GLOBAL (-1)
#define COLUMN_AUX (-2)


typedef struct Event_list
{
    Event* event;
    bool copy;
    struct Event_list* prev;
    struct Event_list* next;
} Event_list;


struct Column_iter
{
    uint32_t version;
    Column* col;
    AAiter* tree_iter;
    Event_list* elist;
};


struct Column
{
    Reltime len;
    bool is_aux;
    uint32_t version;
    Column_iter* edit_iter;
    AAtree* events;
};


static Event_list* new_Event_list(Event_list* nil, Event* event, bool copy);

static Event_list* Event_list_init(Event_list* elist);

static int Event_list_cmp(const Event_list* list1, const Event_list* list2);

static void del_Event_list(Event_list* elist);


static Event_list* new_Event_list(Event_list* nil, Event* event, bool copy)
{
    assert(!(nil == NULL) || (event == NULL));
    assert(!(event == NULL) || (nil == NULL));
    assert(event == NULL || (!copy || EVENT_IS_PG(Event_get_type(event))));
    Event_list* elist = xalloc(Event_list);
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
    elist->copy = copy;
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
    return Reltime_cmp(Event_get_pos(ev1), Event_get_pos(ev2));
}


Column_iter* new_Column_iter(Column* col)
{
    Column_iter* iter = xalloc(Column_iter);
    if (iter == NULL)
    {
        return NULL;
    }
    iter->col = col;
    iter->version = (col != NULL) ? col->version : 0;
    AAtree* events = (col != NULL) ? col->events : NULL;
    iter->tree_iter = new_AAiter(events);
    if (iter->tree_iter == NULL)
    {
        xfree(iter);
        return NULL;
    }
    iter->elist = NULL;
    return iter;
}


void Column_iter_change_col(Column_iter* iter, Column* col)
{
    assert(iter != NULL);
    assert(col != NULL);
    iter->col = col;
    iter->version = col->version;
    AAiter_change_tree(iter->tree_iter, col->events);
    iter->elist = NULL;
    return;
}


Event* Column_iter_get(Column_iter* iter, const Reltime* pos)
{
    assert(iter != NULL);
    assert(pos != NULL);
    if (iter->col == NULL)
    {
        return NULL;
    }
    iter->version = iter->col->version;
    Event* event = &(Event){ .type = EVENT_NONE };
    Reltime_copy(&event->pos, pos);
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    iter->elist = AAiter_get(iter->tree_iter, key);
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
    iter->elist = AAiter_get_next(iter->tree_iter);
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


void del_Column_iter(Column_iter* iter)
{
    if (iter == NULL)
    {
        return;
    }
    del_AAiter(iter->tree_iter);
    xfree(iter);
    return;
}


static bool Column_parse(Column* col,
                         char* str,
                         bool is_global,
                         Event_names* event_names,
                         Read_state* state);


Column* new_Column(Reltime* len)
{
    Column* col = xalloc(Column);
    if (col == NULL)
    {
        return NULL;
    }
    col->version = 1;
    col->is_aux = false;
    col->events = new_AAtree((int (*)(const void*, const void*))Event_list_cmp,
            (void (*)(void*))del_Event_list);
    if (col->events == NULL)
    {
        xfree(col);
        return NULL;
    }
    col->edit_iter = new_Column_iter(col);
    if (col->edit_iter == NULL)
    {
        del_AAtree(col->events);
        xfree(col);
        return NULL;
    }
    if (len != NULL)
    {
        Reltime_copy(&col->len, len);
    }
    else
    {
        Reltime_set(&col->len, INT64_MAX, 0);
    }
    return col;
}


Column* new_Column_aux(Column* old_aux, Column* mod_col, int index)
{
    assert(mod_col != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    Column* aux = new_Column(&mod_col->len);
    if (aux == NULL)
    {
        return NULL;
    }
    aux->is_aux = true;
    Column_iter* iter = new_Column_iter(mod_col);
    if (iter == NULL)
    {
        del_Column(aux);
        return NULL;
    }
    if (old_aux != NULL)
    {
        Column_iter_change_col(iter, old_aux);
        Event* event = Column_iter_get(iter, RELTIME_AUTO);
        while (event != NULL)
        {
            assert(EVENT_IS_PG(Event_get_type(event)));
            if (((Event_pg*)event)->ch_index < index)
            {
                if (!Column_ins(aux, event))
                {
                    del_Column(aux);
                    del_Column_iter(iter);
                    return NULL;
                }
            }
            event = Column_iter_get_next(iter);
        }
    }
    Column_iter_change_col(iter, mod_col);
    Event* event = Column_iter_get(iter, RELTIME_AUTO);
    while (event != NULL)
    {
        if (EVENT_IS_PG(Event_get_type(event)))
        {
            ((Event_pg*)event)->ch_index = index;
            if (!Column_ins(aux, event))
            {
                del_Column(aux);
                del_Column_iter(iter);
                return NULL;
            }
        }
        event = Column_iter_get_next(iter);
    }
    if (old_aux != NULL)
    {
        Column_iter_change_col(iter, old_aux);
        Event* event = Column_iter_get(iter, RELTIME_AUTO);
        while (event != NULL)
        {
            assert(EVENT_IS_PG(Event_get_type(event)));
            if (((Event_pg*)event)->ch_index > index)
            {
                if (!Column_ins(aux, event))
                {
                    del_Column(aux);
                    del_Column_iter(iter);
                    return NULL;
                }
            }
            event = Column_iter_get_next(iter);
        }
    }
    del_Column_iter(iter);
    return aux;
}


Column* new_Column_from_string(Reltime* len,
                               char* str,
                               bool is_global,
                               Event_names* event_names,
                               Read_state* state)
{
    assert(event_names != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Column* col = new_Column(len);
    if (col == NULL)
    {
        return NULL;
    }
    if (!Column_parse(col, str, is_global, event_names, state))
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
                         bool is_global,
                         Event_names* event_names,
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
        str = read_const_char(str, '[', state);
        break_if(state->error);

        Reltime* pos = Reltime_init(RELTIME_AUTO);
        str = read_reltime(str, pos, state);
        break_if(state->error);

        str = read_const_char(str, ',', state);
        str = read_const_char(str, '[', state);
        break_if(state->error);

        char type_str[EVENT_NAME_MAX + 2] = { '\0' };
        str = read_string(str, type_str, EVENT_NAME_MAX + 2, state);
//        str = read_int(str, &type, state);
        break_if(state->error);
        Event_type type = Event_names_get(event_names, type_str);
        if (!EVENT_IS_VALID(type))
        {
            Read_state_set_error(state, "Invalid or unsupported Event type:"
                                        " \"%s\"", type_str);
            return false;
        }
        if (((is_global && EVENT_IS_CHANNEL(type)) ||
                (!is_global && EVENT_IS_GLOBAL(type)))
                && !EVENT_IS_GENERAL(type))
        {
            Read_state_set_error(state,
                     "Incorrect Event type for %s column",
                     is_global ? "global" : "note");
            return false;
        }
        if (!Event_type_is_supported(type))
        {
            Read_state_set_error(state, "Unsupported Event type: %" PRId64
                                        "\n", type);
            return false;
        }

        Event* event = new_Event(type, pos);
        if (event == NULL)
        {
            Read_state_set_error(state, "Couldn't allocate memory for Event");
            return false;
        }
        str = read_const_char(str, ',', state);
        str = Event_read(event, str, state);
        if (state->error)
        {
            del_Event(event);
            return false;
        }
        if (!Column_ins(col, event))
        {
            Read_state_set_error(state, "Couldn't insert Event");
            del_Event(event);
            return false;
        }

        str = read_const_char(str, ']', state);        
        str = read_const_char(str, ']', state);
        break_if(state->error);

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


bool Column_ins(Column* col, Event* event)
{
    assert(col != NULL);
    assert(event != NULL);
    ++col->version;
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* ret = AAtree_get(col->events, key);
    if (ret == NULL || Reltime_cmp(Event_get_pos(event),
            Event_get_pos(ret->next->event)) != 0)
    {
        Event_list* nil = new_Event_list(NULL, NULL, false);
        if (nil == NULL)
        {
            return false;
        }
        Event_list* node = new_Event_list(nil, event, col->is_aux);
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
    Event_list* node = new_Event_list(ret, event, col->is_aux);
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


bool Column_move(Column* col, Event* event, unsigned int index)
{
    assert(col != NULL);
    assert(event != NULL);
    ++col->version;
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* target = AAtree_get(col->events, key);
    if (target == NULL)
    {
        assert(false);
        return false;
    }
    assert(target->event == NULL);
    Event_list* src = NULL;
    Event_list* shifted = NULL;
    Event_list* cur = target->next;
    assert(cur != NULL);
    assert(cur->event != NULL);
    unsigned int idx = 0;
    while (cur != target)
    {
        if (idx == index)
        {
            shifted = cur;
        }
        if (event == cur->event)
        {
            src = cur;
            if (src == shifted)
            {
                return false;
            }
            --idx;
        }
        cur = cur->next;
        ++idx;
    }
    assert(src != NULL);
    src->prev->next = src->next;
    src->next->prev = src->prev;
    if (shifted == NULL)
    {
        src->prev = target->prev;
        assert(target->prev->prev != NULL);
        assert(target->prev->prev != target);
        target->prev->next = src;
        target->prev = src;
    }
    else
    {
        assert(src != shifted);
        src->next = shifted;
        src->prev = shifted->prev;
        assert(src->next != src);
        assert(src->prev != src);
        if (src->prev != NULL)
        {
            src->prev->next = src;
        }
        else
        {
            target->next = src;
        }
        if (src->next != NULL)
        {
            src->next->prev = src;
        }
        else
        {
            target->prev = src;
        }
    }
    return true;
}


bool Column_remove(Column* col, Event* event)
{
    assert(col != NULL);
    assert(event != NULL);
    ++col->version;
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* target = AAtree_get(col->events, key);
    if (target == NULL || Reltime_cmp(Event_get_pos(event),
            Event_get_pos(target->next->event)) != 0)
    {
        return false;
    }
    assert(target->event == NULL);
    Event_list* elist = target->next;
    while (elist != target && elist->event != event)
    {
        elist = elist->next;
        assert(elist != NULL);
    }
    if (elist == target)
    {
        return false;
    }
    Event_list* eprev = elist->prev;
    Event_list* enext = elist->next;
    if (eprev != enext)
    {
        assert(eprev != target || enext != target);
        del_Event(elist->event);
        xfree(elist);
        eprev->next = enext;
        enext->prev = eprev;
        return true;
    }
    assert(eprev == target);
    assert(enext == target);
    target = AAtree_remove(col->events, key);
    assert(target != NULL);
    del_Event_list(target);
    return true;
}


bool Column_remove_row(Column* col, Reltime* pos)
{
    assert(col != NULL);
    assert(pos != NULL);
    ++col->version;
    bool modified = false;
    Event* target = Column_iter_get(col->edit_iter, pos);
    while (target != NULL && Reltime_cmp(Event_get_pos(target), pos) == 0)
    {
        modified = Column_remove(col, target);
        assert(modified);
        target = Column_iter_get(col->edit_iter, pos);
    }
    return modified;
}


bool Column_remove_block(Column* col, Reltime* start, Reltime* end)
{
    assert(col != NULL);
    assert(start != NULL);
    assert(end != NULL);
    ++col->version;
    bool modified = false;
    Event* target = Column_iter_get(col->edit_iter, start);
    while (target != NULL && Reltime_cmp(Event_get_pos(target), end) <= 0)
    {
        modified = Column_remove_row(col, Event_get_pos(target));
        assert(modified);
        target = Column_iter_get(col->edit_iter, start);
    }
    return modified;
}


bool Column_shift_up(Column* col, Reltime* pos, Reltime* len)
{
    assert(col != NULL);
    assert(pos != NULL);
    assert(len != NULL);
    ++col->version;
    bool removed = false;
    Reltime* del_end = Reltime_set(RELTIME_AUTO, 0, 1);
    del_end = Reltime_sub(del_end, len, del_end);
    del_end = Reltime_add(del_end, pos, del_end);
    removed = Column_remove_block(col, pos, del_end);
    Event* target = Column_iter_get(col->edit_iter, pos);
    while (target != NULL)
    {
        Reltime* ev_pos = Event_get_pos(target);
        Reltime_sub(ev_pos, ev_pos, len);
        target = Column_iter_get_next(col->edit_iter);
    }
    return removed;
}


void Column_shift_down(Column* col, Reltime* pos, Reltime* len)
{
    assert(col != NULL);
    assert(pos != NULL);
    assert(len != NULL);
    ++col->version;
    Event* target = Column_iter_get(col->edit_iter, pos);
    while (target != NULL)
    {
        Reltime* ev_pos = Event_get_pos(target);
        Reltime_add(ev_pos, ev_pos, len);
        target = Column_iter_get_next(col->edit_iter);
    }
    return;
}


void Column_clear(Column* col)
{
    assert(col != NULL);
    ++col->version;
    AAtree_clear(col->events);
    Column_iter_change_col(col->edit_iter, col);
    return;
}


void Column_set_length(Column* col, Reltime* len)
{
    assert(col != NULL);
    assert(len != NULL);
    Reltime_copy(&col->len, len);
    return;
}


Reltime* Column_length(Column* col)
{
    assert(col != NULL);
    return &col->len;
}


void del_Column(Column* col)
{
    if (col == NULL)
    {
        return;
    }
    del_AAtree(col->events);
    del_Column_iter(col->edit_iter);
    xfree(col);
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
        xfree(cur);
        cur = next;
    }
    assert(cur == elist);
    xfree(cur);
    return;
}


