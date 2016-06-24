

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/sheet/Column.h>

#include <debug/assert.h>
#include <mathnum/Tstamp.h>
#include <memory.h>
#include <player/Event_names.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Column
{
    Tstamp len;
    uint32_t version;
    Column_iter* edit_iter;
    AAtree* triggers;
};


static Trigger_list* new_Trigger_list(Trigger_list* nil, Trigger* trigger);

static Trigger_list* Trigger_list_init(Trigger_list* trlist);

static int Trigger_list_cmp(const Trigger_list* list1, const Trigger_list* list2);

static void del_Trigger_list(Trigger_list* trlist);


static Trigger_list* new_Trigger_list(Trigger_list* nil, Trigger* trigger)
{
    assert(!(nil == NULL) || (trigger == NULL));
    assert(!(trigger == NULL) || (nil == NULL));

    Trigger_list* trlist = memory_alloc_item(Trigger_list);
    if (trlist == NULL)
        return NULL;

    if (nil == NULL)
    {
        assert(trigger == NULL);
        trlist->trigger = NULL;
        trlist->prev = trlist->next = trlist;
    }
    else
    {
        assert(trigger != NULL);
        trlist->trigger = trigger;
        trlist->prev = trlist->next = nil;
    }

    return trlist;
}


static Trigger_list* Trigger_list_init(Trigger_list* trlist)
{
    assert(trlist != NULL);
    trlist->prev = trlist->next = trlist;
    return trlist;
}


static int Trigger_list_cmp(const Trigger_list* list1, const Trigger_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);

    Trigger* ev1 = list1->next->trigger;
    Trigger* ev2 = list2->next->trigger;
    assert(ev1 != NULL);
    assert(ev2 != NULL);

    return Tstamp_cmp(Trigger_get_pos(ev1), Trigger_get_pos(ev2));
}


Column_iter* new_Column_iter(Column* col)
{
    Column_iter* iter = memory_alloc_item(Column_iter);
    if (iter == NULL)
        return NULL;

    Column_iter_init(iter);

    if (col != NULL)
    {
        iter->version = col->version;
        iter->col = col;
        iter->tree_iter.tree = col->triggers;
    }

    return iter;
}


void Column_iter_init(Column_iter* iter)
{
    assert(iter != NULL);

    iter->version = 0;
    iter->col = NULL;
    iter->tree_iter = *AAITER_AUTO;
    iter->trlist = NULL;

    return;
}


void Column_iter_change_col(Column_iter* iter, Column* col)
{
    assert(iter != NULL);
    assert(col != NULL);

    iter->col = col;
    iter->version = col->version;
    AAiter_init(&iter->tree_iter, col->triggers);
    iter->trlist = NULL;

    return;
}


Trigger* Column_iter_get(Column_iter* iter, const Tstamp* pos)
{
    assert(iter != NULL);
    assert(pos != NULL);

    iter->trlist = Column_iter_get_row(iter, pos);
    if (iter->trlist == NULL)
        return NULL;

    assert(iter->trlist->trigger == NULL);
    assert(iter->trlist->next != iter->trlist);
    iter->trlist = iter->trlist->next;
    assert(iter->trlist->trigger != NULL);

    return iter->trlist->trigger;
}


Trigger_list* Column_iter_get_row(Column_iter* iter, const Tstamp* pos)
{
    assert(iter != NULL);
    assert(pos != NULL);

    if (iter->col == NULL)
        return NULL;

    iter->version = iter->col->version;
    Trigger* trigger = &(Trigger){ .type = Event_NONE };
    Tstamp_copy(&trigger->pos, pos);
    Trigger_list* key = Trigger_list_init(&(Trigger_list){ .trigger = trigger });
    iter->trlist = AAiter_get_at_least(&iter->tree_iter, key);

    return iter->trlist;
}


Trigger* Column_iter_get_next(Column_iter* iter)
{
    assert(iter != NULL);

    if (iter->trlist == NULL || iter->col == NULL)
        return NULL;

    if (iter->version != iter->col->version)
    {
        iter->trlist = NULL;
        iter->version = iter->col->version;
        return NULL;
    }

    assert(iter->trlist->trigger != NULL);
    assert(iter->trlist != iter->trlist->next);

    iter->trlist = iter->trlist->next;
    if (iter->trlist->trigger != NULL)
        return iter->trlist->trigger;

    iter->trlist = Column_iter_get_next_row(iter);
    if (iter->trlist == NULL)
        return NULL;

    assert(iter->trlist->trigger == NULL);
    assert(iter->trlist->next != iter->trlist);
    iter->trlist = iter->trlist->next;
    assert(iter->trlist->trigger != NULL);

    return iter->trlist->trigger;
}


Trigger_list* Column_iter_get_next_row(Column_iter* iter)
{
    assert(iter != NULL);

    if (iter->trlist == NULL || iter->col == NULL)
        return NULL;

    if (iter->version != iter->col->version)
    {
        iter->trlist = NULL;
        iter->version = iter->col->version;
        return NULL;
    }

    iter->trlist = AAiter_get_next(&iter->tree_iter);

    return iter->trlist;
}


void del_Column_iter(Column_iter* iter)
{
    if (iter == NULL)
        return;

    memory_free(iter);
    return;
}


static bool Column_parse(Column* col, Streader* sr, const Event_names* event_names);


Column* new_Column(const Tstamp* len)
{
    Column* col = memory_alloc_item(Column);
    if (col == NULL)
        return NULL;

    col->version = 1;
    col->triggers = new_AAtree((int (*)(const void*, const void*))Trigger_list_cmp,
            (void (*)(void*))del_Trigger_list);
    if (col->triggers == NULL)
    {
        memory_free(col);
        return NULL;
    }

    col->edit_iter = new_Column_iter(col);
    if (col->edit_iter == NULL)
    {
        del_AAtree(col->triggers);
        memory_free(col);
        return NULL;
    }

    if (len != NULL)
        Tstamp_copy(&col->len, len);
    else
        Tstamp_set(&col->len, INT64_MAX, 0);

    return col;
}


Column* new_Column_from_string(
        Streader* sr, const Tstamp* len, const Event_names* event_names)
{
    assert(sr != NULL);
    assert(event_names != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Column* col = new_Column(len);
    if (col == NULL)
        return NULL;

    if (!Column_parse(col, sr, event_names))
    {
        del_Column(col);
        return NULL;
    }

    return col;
}


typedef struct Read_trigger_data
{
    Column* col;
    const Event_names* event_names;
} Read_trigger_data;

static bool read_trigger(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    ignore(index);
    assert(userdata != NULL);

    Read_trigger_data* rtdata = userdata;

    Trigger* trigger = new_Trigger_from_string(sr, rtdata->event_names);
    if (trigger == NULL || !Column_ins(rtdata->col, trigger))
    {
        del_Trigger(trigger);
        return false;
    }

    return true;
}

static bool Column_parse(Column* col, Streader* sr, const Event_names* event_names)
{
    assert(col != NULL);
    assert(sr != NULL);
    assert(event_names != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Streader_has_data(sr))
    {
        // An empty Column has no properties, so we're done.
        return true;
    }

    Read_trigger_data rtdata = { col, event_names };
    return Streader_read_list(sr, read_trigger, &rtdata);
}


bool Column_ins(Column* col, Trigger* trigger)
{
    assert(col != NULL);
    assert(trigger != NULL);

    ++col->version;
    Trigger_list* key = Trigger_list_init(&(Trigger_list){ .trigger = trigger });
    Trigger_list* ret = AAtree_get_at_least(col->triggers, key);
    if (ret == NULL || Tstamp_cmp(Trigger_get_pos(trigger),
            Trigger_get_pos(ret->next->trigger)) != 0)
    {
        Trigger_list* nil = new_Trigger_list(NULL, NULL);
        if (nil == NULL)
            return false;

        Trigger_list* node = new_Trigger_list(nil, trigger);
        if (node == NULL)
        {
            del_Trigger_list(nil);
            return false;
        }

        nil->prev = nil->next = node;
        node->prev = node->next = nil;

        if (!AAtree_ins(col->triggers, nil))
        {
            del_Trigger_list(nil);
            del_Trigger_list(node);
            return false;
        }

        return true;
    }

    assert(ret->next != ret);
    assert(ret->prev != ret);
    assert(ret->trigger == NULL);

    Trigger_list* node = new_Trigger_list(ret, trigger);
    if (node == NULL)
        return false;

    node->prev = ret->prev;
    node->next = ret;
    ret->prev->next = node;
    ret->prev = node;

    return true;
}


void del_Column(Column* col)
{
    if (col == NULL)
        return;

    del_AAtree(col->triggers);
    del_Column_iter(col->edit_iter);
    memory_free(col);

    return;
}


static void del_Trigger_list(Trigger_list* trlist)
{
    if (trlist == NULL)
        return;

    assert(trlist->trigger == NULL);
    Trigger_list* cur = trlist->next;
    assert(cur->trigger != NULL);

    while (cur->trigger != NULL)
    {
        Trigger_list* next = cur->next;
        assert(cur->trigger != NULL);
        del_Trigger(cur->trigger);
        memory_free(cur);
        cur = next;
    }

    assert(cur == trlist);
    memory_free(cur);

    return;
}


