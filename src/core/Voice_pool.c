

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
#include <stdbool.h>

#include <Voice_pool.h>

#include <xmemory.h>


#define HEAP_PARENT(i) (((i) - 1) / 2)
#define HEAP_LEFT(i)   (2 * (i) + 1)
#define HEAP_RIGHT(i)  (2 * (i) + 2)


/**
 * Moves a modified key into its proper position in the heap.
 * 
 * The tree must satisfy the minimum heap property apart from the given index.
 *
 * \param pool    The Voice pool -- must not be \c NULL.
 * \param index   The index of the key -- must be less than the pool size.
 *
 * \return   The target index of the modified key.
 */
// static uint16_t heap_mod_key(Voice_pool* pool, uint16_t index);


/**
 * Arranges the Voice pool to satisfy the minimum heap property.
 *
 * \param pool   The Voice pool -- must not be \c NULL.
 */
static void heap_build(Voice_pool* pool);


/**
 * Performs a downheap operation starting at specified index.
 *
 * The subtrees must satisfy the minimum heap property.
 *
 * \param pool    The Voice pool -- must not be \c NULL.
 * \param index   The index of the key -- must be less than the pool size.
 *
 * \return   The target index of the modified key.
 */
static uint16_t downheap(Voice_pool* pool, uint16_t index);


/**
 * Performs an upheap operation starting at specified index.
 *
 * The tree must satisfy the minimum heap property apart from the given index.
 *
 * \param pool    The Voice pool -- must not be \c NULL.
 * \param index   The index of the key -- must be less than the pool size.
 *
 * \return   The target index of the modified key.
 */
// static uint16_t upheap(Voice_pool* pool, uint16_t index);


Voice_pool* new_Voice_pool(uint16_t size, uint8_t events)
{
    assert(size > 0);
    assert(events > 0);
    Voice_pool* pool = xalloc(Voice_pool);
    if (pool == NULL)
    {
        return NULL;
    }
    pool->size = size;
    pool->events = events;
    pool->voices = xnalloc(Voice, size);
    if (pool->voices == NULL)
    {
        xfree(pool);
        return NULL;
    }
    for (int i = 0; i < size; ++i)
    {
        pool->voices[i] = new_Voice(events);
        if (pool->voices[i] == NULL)
        {
            for (--i; i >= 0; --i)
            {
                del_Voice(pool->voices[i]);
            }
            xfree(pool->voices);
            xfree(pool);
            return NULL;
        }
    }
    return pool;
}


bool Voice_pool_resize(Voice_pool* pool, uint16_t size)
{
    assert(pool != NULL);
    assert(size > 0);
    uint16_t new_size = size;
    if (new_size == pool->size)
    {
        return true;
    }
    for (uint16_t i = new_size; i < pool->size; ++i)
    {
        del_Voice(pool->voices[i]);
        pool->voices[i] = NULL;
    }
    if (new_size < pool->size)
    {
        pool->size = new_size;
    }
    Voice** new_voices = xrealloc(Voice*, new_size, pool->voices);
    if (new_voices == NULL)
    {
        return false;
    }
    pool->voices = new_voices;
    for (uint16_t i = pool->size; i < new_size; ++i)
    {
        pool->voices[i] = NULL;
    }
    for (uint16_t i = pool->size; i < new_size; ++i)
    {
        pool->voices[i] = new_Voice(pool->events);
        if (pool->voices[i] == NULL)
        {
            for (--i; i >= pool->size; --i)
            {
                del_Voice(pool->voices[i]);
                pool->voices[i] = NULL;
            }
            return false;
        }
    }
    pool->size = new_size;
    return true;
}


uint16_t Voice_pool_get_size(Voice_pool* pool)
{
    assert(pool != NULL);
    return pool->size;
}


Voice* Voice_pool_get_voice(Voice_pool* pool,
        Voice* voice,
        uint64_t id)
{
    assert(pool != NULL);
    if (voice == NULL)
    {
        static uint64_t running_id = 0;
        downheap(pool, 0);
        Voice* new_voice = pool->voices[0];
        new_voice->id = running_id;
        new_voice->prio = VOICE_PRIO_INACTIVE;
        Event_queue_clear(new_voice->events);
        ++running_id;
        return new_voice;
    }
    if (voice->id == id)
    {
        return voice;
    }
    return NULL;
}


uint16_t Voice_pool_mix(Voice_pool* pool,
        uint32_t amount,
        uint32_t offset,
        uint32_t freq)
{
    assert(pool != NULL);
    assert(freq > 0);
    uint16_t active_voices = 0;
    for (uint16_t i = 0; i < pool->size; ++i)
    {
        if (pool->voices[i]->prio != VOICE_PRIO_INACTIVE)
        {
            Voice_mix(pool->voices[i], amount, offset, freq);
            ++active_voices;
        }
    }
    heap_build(pool);
    return active_voices;
}


void Voice_pool_reset(Voice_pool* pool)
{
    for (uint16_t i = 0; i < pool->size; ++i)
    {
        Voice_reset(pool->voices[i]);
    }
    return;
}


void del_Voice_pool(Voice_pool* pool)
{
    assert(pool != NULL);
    for (uint16_t i = 0; i < pool->size; ++i)
    {
        del_Voice(pool->voices[i]);
        pool->voices[i] = NULL;
    }
    xfree(pool->voices);
    xfree(pool);
    return;
}


#if 0
static uint16_t heap_mod_key(Voice_pool* pool, uint16_t index)
{
    assert(pool != NULL);
    assert(index < pool->size);
    index = upheap(pool, index);
    return downheap(pool, index);
}
#endif


static void heap_build(Voice_pool* pool)
{
    assert(pool != NULL);
    uint16_t index = HEAP_PARENT(pool->size + 1);
    while (index-- > 0)
    {
        downheap(pool, index);
    }
    return;
}


static uint16_t downheap(Voice_pool* pool, uint16_t index)
{
    assert(pool != NULL);
    assert(index < pool->size);
    uint16_t smallest = index;
    uint32_t left = HEAP_LEFT(index);
    while (left < pool->size)
    {
        if (Voice_cmp(pool->voices[left], pool->voices[smallest]) < 0)
        {
            smallest = left;
        }
        uint32_t right = HEAP_RIGHT(index);
        if (right < pool->size && Voice_cmp(pool->voices[right], pool->voices[smallest]) < 0)
        {
            smallest = right;
        }
        if (index == smallest)
        {
            return index;
        }
        Voice* tmp = pool->voices[index];
        pool->voices[index] = pool->voices[smallest];
        pool->voices[smallest] = tmp;
        pool->voices[index]->pool_index = index;
        pool->voices[smallest]->pool_index = smallest;
        index = smallest;
        left = HEAP_LEFT(index);
    }
    return index;
}


#if 0
static uint16_t upheap(Voice_pool* pool, uint16_t index)
{
    assert(pool != NULL);
    assert(index < pool->size);
    uint16_t parent = HEAP_PARENT(index);
    while (index > 0 && Voice_cmp(pool->voices[index], pool->voices[parent]) < 0)
    {
        assert(parent < index);
        Voice* tmp = pool->voices[index];
        pool->voices[index] = pool->voices[parent];
        pool->voices[parent] = tmp;
        pool->voices[index]->pool_index = index;
        pool->voices[parent]->pool_index = parent;
        index = parent;
        parent = HEAP_PARENT(index);
    }
    return index;
}
#endif


