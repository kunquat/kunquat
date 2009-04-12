

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <Reltime.h>

#include <Column.h>

#include <xmemory.h>


static Event_list* new_Event_list(Event_list* nil, Event* event);

static Event_list* Event_list_init(Event_list* elist);

static int Event_list_cmp(Event_list* list1, Event_list* list2);

static void del_Event_list(Event_list* elist);


static Event_list* new_Event_list(Event_list* nil, Event* event)
{
    assert(!(nil == NULL) || (event == NULL));
    assert(!(event == NULL) || (nil == NULL));
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
    return elist;
}


static Event_list* Event_list_init(Event_list* elist)
{
    assert(elist != NULL);
    elist->prev = elist->next = elist;
    return elist;
}


static int Event_list_cmp(Event_list* list1, Event_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);
    Event* ev1 = list1->next->event;
    Event* ev2 = list2->next->event;
    assert(ev1 != NULL);
    assert(ev2 != NULL);
    return Reltime_cmp(Event_pos(ev1), Event_pos(ev2));
}


Column* new_Column(Reltime* len)
{
    Column* col = xalloc(Column);
    if (col == NULL)
    {
        return NULL;
    }
    col->events = new_AAtree((int (*)(void*, void*))Event_list_cmp,
            (void (*)(void*))del_Event_list);
    if (col->events == NULL)
    {
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
    col->last_elist = NULL;
    col->last_elist_from_host = NULL;
    return col;
}


bool Column_ins(Column* col, Event* event)
{
    assert(col != NULL);
    assert(event != NULL);
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* ret = AAtree_get(col->events, key, 1);
    if (ret == NULL || Reltime_cmp(Event_pos(event),
            Event_pos(ret->next->event)) != 0)
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


Event* Column_get(Column* col, const Reltime* pos)
{
    assert(col != NULL);
    assert(pos != NULL);
    Event* event = &(Event){ .type = EVENT_TYPE_NONE };
    Reltime_copy(&event->pos, pos);
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    col->last_elist = AAtree_get(col->events, key, 0);
    if (col->last_elist == NULL)
    {
        return NULL;
    }
    assert(col->last_elist->event == NULL);
    assert(col->last_elist->next != col->last_elist);
    col->last_elist = col->last_elist->next;
    assert(col->last_elist->event != NULL);
    return col->last_elist->event;
}


Event* Column_get_edit(Column* col, const Reltime* pos)
{
    assert(col != NULL);
    assert(pos != NULL);
    Event* event = &(Event){ .type = EVENT_TYPE_NONE };
    Reltime_copy(&event->pos, pos);
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    col->last_elist_from_host = AAtree_get(col->events, key, 1);
    if (col->last_elist_from_host == NULL)
    {
        return NULL;
    }
    assert(col->last_elist_from_host->event == NULL);
    assert(col->last_elist_from_host->next != col->last_elist_from_host);
    col->last_elist_from_host = col->last_elist_from_host->next;
    assert(col->last_elist_from_host->event != NULL);
    return col->last_elist_from_host->event;
}


Event* Column_get_next(Column* col)
{
    assert(col != NULL);
    if (col->last_elist == NULL)
    {
        return NULL;
    }
    assert(col->last_elist->event != NULL);
    assert(col->last_elist != col->last_elist->next);
    col->last_elist = col->last_elist->next;
    if (col->last_elist->event != NULL)
    {
        return col->last_elist->event;
    }
    col->last_elist = AAtree_get_next(col->events, 0);
    if (col->last_elist == NULL)
    {
        return NULL;
    }
    assert(col->last_elist->event == NULL);
    assert(col->last_elist->next != col->last_elist);
    col->last_elist = col->last_elist->next;
    assert(col->last_elist->event != NULL);
    return col->last_elist->event;
}


Event* Column_get_next_edit(Column* col)
{
    assert(col != NULL);
    if (col->last_elist_from_host == NULL)
    {
        return NULL;
    }
    assert(col->last_elist_from_host->event != NULL);
    assert(col->last_elist_from_host != col->last_elist_from_host->next);
    col->last_elist_from_host = col->last_elist_from_host->next;
    if (col->last_elist_from_host->event != NULL)
    {
        return col->last_elist_from_host->event;
    }
    col->last_elist_from_host = AAtree_get_next(col->events, 1);
    if (col->last_elist_from_host == NULL)
    {
        return NULL;
    }
    assert(col->last_elist_from_host->event == NULL);
    assert(col->last_elist_from_host->next != col->last_elist_from_host);
    col->last_elist_from_host = col->last_elist_from_host->next;
    assert(col->last_elist_from_host->event != NULL);
    return col->last_elist_from_host->event;
}


bool Column_move(Column* col, Event* event, unsigned int index)
{
    assert(col != NULL);
    assert(event != NULL);
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* target = AAtree_get(col->events, key, 1);
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
    Event_list* key = Event_list_init(&(Event_list){ .event = event });
    Event_list* target = AAtree_get(col->events, key, 1);
    if (target == NULL || Reltime_cmp(Event_pos(event),
            Event_pos(target->next->event)) != 0)
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
    bool modified = false;
    Event* target = Column_get_edit(col, pos);
    while (target != NULL && Reltime_cmp(Event_pos(target), pos) == 0)
    {
        modified = Column_remove(col, target);
        assert(modified);
        target = Column_get_edit(col, pos);
    }
    return modified;
}


bool Column_remove_block(Column* col, Reltime* start, Reltime* end)
{
    assert(col != NULL);
    assert(start != NULL);
    assert(end != NULL);
    bool modified = false;
    Event* target = Column_get_edit(col, start);
    while (target != NULL && Reltime_cmp(Event_pos(target), end) <= 0)
    {
        modified = Column_remove_row(col, Event_pos(target));
        assert(modified);
        target = Column_get_edit(col, start);
    }
    return modified;
}


bool Column_shift_up(Column* col, Reltime* pos, Reltime* len)
{
    assert(col != NULL);
    assert(pos != NULL);
    assert(len != NULL);
    bool removed = false;
    Reltime* del_end = Reltime_set(RELTIME_AUTO, 0, 1);
    del_end = Reltime_sub(del_end, len, del_end);
    del_end = Reltime_add(del_end, pos, del_end);
    removed = Column_remove_block(col, pos, del_end);
    Event* target = Column_get_edit(col, pos);
    while (target != NULL)
    {
        Reltime* ev_pos = Event_pos(target);
        Reltime_sub(ev_pos, ev_pos, len);
        target = Column_get_next_edit(col);
    }
    return removed;
}


void Column_shift_down(Column* col, Reltime* pos, Reltime* len)
{
    assert(col != NULL);
    assert(pos != NULL);
    assert(len != NULL);
    Event* target = Column_get_edit(col, pos);
    while (target != NULL)
    {
        Reltime* ev_pos = Event_pos(target);
        Reltime_add(ev_pos, ev_pos, len);
        target = Column_get_next_edit(col);
    }
    return;
}


void Column_clear(Column* col)
{
    assert(col != NULL);
    col->last_elist = NULL;
    col->last_elist_from_host = NULL;
    AAtree_clear(col->events);
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
    assert(col != NULL);
    del_AAtree(col->events);
    xfree(col);
    return;
}


static void del_Event_list(Event_list* elist)
{
    assert(elist != NULL);
    assert(elist->event == NULL);
    Event_list* cur = elist->next;
    assert(cur->event != NULL);
    while (cur->event != NULL)
    {
        Event_list* next = cur->next;
        assert(cur->event != NULL);
        del_Event(cur->event);
        xfree(cur);
        cur = next;
    }
    assert(cur == elist);
    xfree(cur);
    return;
}


