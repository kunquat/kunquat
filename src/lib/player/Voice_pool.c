

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


#include <player/Voice_pool.h>

#include <debug/assert.h>
#include <memory.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


Voice_pool* new_Voice_pool(int size)
{
    assert(size >= 0);

    Voice_pool* pool = memory_alloc_item(Voice_pool);
    if (pool == NULL)
        return NULL;

    pool->size = size;
    pool->state_size = 0;
    pool->new_group_id = 0;
    pool->voices = NULL;
    pool->group_iter_offset = 0;
    pool->group_iter = *VOICE_GROUP_AUTO;

    if (size > 0)
    {
        pool->voices = memory_alloc_items(Voice, size);
        if (pool->voices == NULL)
        {
            memory_free(pool);
            return NULL;
        }
    }

    for (int i = 0; i < size; ++i)
    {
        pool->voices[i] = new_Voice();
        if (pool->voices[i] == NULL)
        {
            for (--i; i >= 0; --i)
            {
                del_Voice(pool->voices[i]);
            }
            memory_free(pool->voices);
            memory_free(pool);
            return NULL;
        }
    }

    return pool;
}


bool Voice_pool_reserve_state_space(Voice_pool* pool, int32_t state_size)
{
    assert(pool != NULL);
    assert(state_size >= 0);

    if (state_size <= pool->state_size)
        return true;

    for (uint16_t i = 0; i < pool->size; ++i)
    {
        if (!Voice_reserve_state_space(pool->voices[i], state_size))
            return false;
    }

    pool->state_size = state_size;
    return true;
}


bool Voice_pool_resize(Voice_pool* pool, int size)
{
    assert(pool != NULL);
    assert(size > 0);

    int new_size = size;
    if (new_size == pool->size)
        return true;

    // Remove excess voices if any
    for (int i = new_size; i < pool->size; ++i)
    {
        del_Voice(pool->voices[i]);
        pool->voices[i] = NULL;
    }

    if (new_size < pool->size)
        pool->size = new_size;

    // Handle 0 voices
    if (new_size == 0)
    {
        memory_free(pool->voices);
        pool->voices = NULL;
        return true;
    }

    // Resize voice array
    Voice** new_voices = memory_realloc_items(Voice*, new_size, pool->voices);
    if (new_voices == NULL)
        return false;

    pool->voices = new_voices;

    // Sanitise new fields if any
    for (int i = pool->size; i < new_size; ++i)
        pool->voices[i] = NULL;

    // Allocate new voices
    for (int i = pool->size; i < new_size; ++i)
    {
        pool->voices[i] = new_Voice();
        if (pool->voices[i] == NULL ||
                !Voice_reserve_state_space(pool->voices[i], pool->state_size))
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


int Voice_pool_get_size(const Voice_pool* pool)
{
    assert(pool != NULL);
    return pool->size;
}


uint64_t Voice_pool_new_group_id(Voice_pool* pool)
{
    assert(pool != NULL);
    ++pool->new_group_id;
    return pool->new_group_id;
}


Voice* Voice_pool_get_voice(Voice_pool* pool, Voice* voice, uint64_t id)
{
    assert(pool != NULL);

    if (pool->size == 0)
        return NULL;

    if (voice == NULL)
    {
        // Find a voice of lowest priority available
        Voice* new_voice = pool->voices[0];
        for (uint16_t i = 1; i < pool->size; ++i)
        {
            if (Voice_cmp(pool->voices[i], new_voice) < 0)
                new_voice = pool->voices[i];
        }

        // Pre-init the voice
        static uint64_t running_id = 1;
        new_voice->id = running_id;
        new_voice->prio = VOICE_PRIO_INACTIVE;
        ++running_id;

        return new_voice;
    }

    if (voice->id == id)
        return voice;

    return NULL;
}


static uint64_t get_voice_group_prio(const Voice* voice)
{
    // Overflow group ID 0 to maximum so that inactive voices are placed last
    return Voice_get_group_id(voice) - 1;
}


static void Voice_pool_sort_groups(Voice_pool* pool)
{
    assert(pool != NULL);

    // Simple insertion sort based on group IDs
    for (uint16_t i = 1; i < pool->size; ++i)
    {
        Voice* current = pool->voices[i];
        uint16_t target_index = i;

        for (; target_index > 0; --target_index)
        {
            Voice* prev = pool->voices[target_index - 1];
            if (get_voice_group_prio(prev) <= get_voice_group_prio(current))
                break;

            pool->voices[target_index] = prev;
        }

        pool->voices[target_index] = current;
    }

    return;
}


Voice_group* Voice_pool_start_group_iteration(Voice_pool* pool)
{
    assert(pool != NULL);

    Voice_pool_sort_groups(pool);

    Voice_group_init(&pool->group_iter, pool->voices, 0, pool->size);
    pool->group_iter_offset = Voice_group_get_size(&pool->group_iter);

    if (Voice_group_get_size(&pool->group_iter) == 0)
        return NULL;

    return &pool->group_iter;
}


Voice_group* Voice_pool_get_next_group(Voice_pool* pool)
{
    assert(pool != NULL);

    if (pool->group_iter_offset >= pool->size)
        return NULL;

    Voice_group_init(
            &pool->group_iter, pool->voices, pool->group_iter_offset, pool->size);
    pool->group_iter_offset += Voice_group_get_size(&pool->group_iter);

    if (Voice_group_get_size(&pool->group_iter) == 0)
        return NULL;

    return &pool->group_iter;
}


void Voice_pool_reset(Voice_pool* pool)
{
    assert(pool != NULL);

    for (uint16_t i = 0; i < pool->size; ++i)
        Voice_reset(pool->voices[i]);

    return;
}


void del_Voice_pool(Voice_pool* pool)
{
    if (pool == NULL)
        return;

    for (uint16_t i = 0; i < pool->size; ++i)
    {
        del_Voice(pool->voices[i]);
        pool->voices[i] = NULL;
    }
    memory_free(pool->voices);
    memory_free(pool);

    return;
}


