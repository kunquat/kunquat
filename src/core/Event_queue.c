

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

#include <Event_queue.h>

#include <xmemory.h>


Event_queue* new_Event_queue(int size)
{
    assert(size > 0);
    Event_queue* q = xalloc(Event_queue);
    if (q == NULL)
    {
        return NULL;
    }
    q->queue = xnalloc(Event_queue_node, size);
    if (q->queue == NULL)
    {
        xfree(q);
        return NULL;
    }
    q->size = size;
    q->start = 0;
    q->count = 0;
    return q;
}


bool Event_queue_ins(Event_queue* q, Event* event, uint32_t pos)
{
    assert(q != NULL);
    assert(event != NULL);
    if (q->count >= q->size)
    {
        assert(q->count == q->size);
        return false;
    }
    int i = q->count;
    for (i = q->count; i > 0; --i)
    {
        int cur = (q->start + i) % q->size;
        int prev = (q->start + (i - 1)) % q->size;
        if (q->queue[prev].pos <= pos)
        {
            break;
        }
        q->queue[cur].pos = q->queue[prev].pos;
        q->queue[cur].event = q->queue[prev].event;
    }
    q->queue[(q->start + i) % q->size].pos = pos;
    q->queue[(q->start + i) % q->size].event = event;
    ++q->count;
    return true;
}


bool Event_queue_get(Event_queue* q, Event** dest, uint32_t* pos)
{
    assert(q != NULL);
    assert(dest != NULL);
    assert(pos != NULL);
    if (q->count <= 0)
    {
        assert(q->count == 0);
        return false;
    }
    *dest = q->queue[q->start].event;
    *pos = q->queue[q->start].pos;
    --q->count;
    q->start = (q->start + 1) % q->size;
    return true;
}


bool Event_queue_peek(Event_queue* q, int index, Event** dest, uint32_t* pos)
{
    assert(q != NULL);
    assert(index >= 0);
    assert(dest != NULL);
    assert(pos != NULL);
    if (index >= q->count)
    {
        return false;
    }
    *dest = q->queue[(q->start + index) % q->size].event;
    *pos = q->queue[(q->start + index) % q->size].pos;
    return true;
}


void Event_queue_clear(Event_queue* q)
{
    assert(q != NULL);
    q->start = 0;
    q->count = 0;
    return;
}


bool Event_queue_resize(Event_queue* q, int new_size)
{
    assert(q != NULL);
    assert(new_size > 0);
    if (new_size != q->size)
    {
        Event_queue_node* new_queue = xrealloc(Event_queue_node, new_size, q->queue);
        if (new_queue == NULL)
        {
            return false;
        }
        q->queue = new_queue;
        q->size = new_size;
    }
    Event_queue_clear(q);
    return true;
}


void del_Event_queue(Event_queue* q)
{
    assert(q != NULL);
    xfree(q->queue);
    xfree(q);
    return;
}


