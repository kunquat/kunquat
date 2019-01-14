

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
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
#include <player/Voice_work_buffers.h>
#include <threads/Mutex.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


struct Voice_pool
{
    int size;
    int32_t state_size;
    uint64_t new_group_id;
    Voice** voices;
    Voice_work_buffers* voice_wbs;

    int group_iter_offset;

    Mutex group_iter_lock;
};


Voice_pool* new_Voice_pool(int size)
{
    rassert(size >= 0);

    Voice_pool* pool = memory_alloc_item(Voice_pool);
    if (pool == NULL)
        return NULL;

    pool->size = size;
    pool->state_size = 0;
    pool->new_group_id = 0;
    pool->voices = NULL;
    pool->voice_wbs = NULL;
    pool->group_iter_offset = 0;

    pool->group_iter_lock = *MUTEX_AUTO;

    pool->voice_wbs = new_Voice_work_buffers();
    if (pool->voice_wbs == NULL)
    {
        del_Voice_pool(pool);
        return NULL;
    }

    if (size > 0)
    {
        pool->voices = memory_alloc_items(Voice, size);
        if (pool->voices == NULL)
        {
            del_Voice_pool(pool);
            return NULL;
        }

        for (int i = 0; i < size; ++i)
            pool->voices[i] = NULL;

        for (int i = 0; i < size; ++i)
        {
            pool->voices[i] = new_Voice();
            if (pool->voices[i] == NULL)
            {
                del_Voice_pool(pool);
                return NULL;
            }
        }
    }

#ifdef ENABLE_THREADS
    Mutex_init(&pool->group_iter_lock);
#endif

    return pool;
}


bool Voice_pool_reserve_state_space(Voice_pool* pool, int32_t state_size)
{
    rassert(pool != NULL);
    rassert(state_size >= 0);

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


int32_t Voice_pool_get_work_buffer_size(const Voice_pool* pool)
{
    rassert(pool != NULL);
    return Voice_work_buffers_get_buffer_size(pool->voice_wbs);
}


bool Voice_pool_reserve_work_buffers(Voice_pool* pool, int32_t buf_size)
{
    rassert(pool != NULL);
    rassert(buf_size >= 0);
    rassert(buf_size <= VOICE_WORK_BUFFER_SIZE_MAX);

    if (!Voice_work_buffers_allocate_space(pool->voice_wbs, pool->size, buf_size))
        return false;

    for (int i = 0; i < pool->size; ++i)
    {
        Voice_set_work_buffer(
                pool->voices[i],
                Voice_work_buffers_get_buffer_mut(pool->voice_wbs, i));
    }

    return true;
}


bool Voice_pool_resize(Voice_pool* pool, int size)
{
    rassert(pool != NULL);
    rassert(size > 0);

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

    // Allocate space for Voice work buffers if needed
    const int32_t voice_wb_size = Voice_work_buffers_get_buffer_size(pool->voice_wbs);
    if (voice_wb_size > 0)
    {
        if (!Voice_work_buffers_allocate_space(pool->voice_wbs, new_size, voice_wb_size))
            return false;

        for (int i = 0; i < new_size; ++i)
            Voice_set_work_buffer(
                    pool->voices[i],
                    Voice_work_buffers_get_buffer_mut(pool->voice_wbs, i));
    }

    pool->size = new_size;
    return true;
}


int Voice_pool_get_size(const Voice_pool* pool)
{
    rassert(pool != NULL);
    return pool->size;
}


uint64_t Voice_pool_new_group_id(Voice_pool* pool)
{
    rassert(pool != NULL);
    ++pool->new_group_id;
    return pool->new_group_id;
}


Voice* Voice_pool_get_voice(Voice_pool* pool, uint64_t group_id)
{
    rassert(pool != NULL);
    rassert(group_id != 0);

    if (pool->size == 0)
        return NULL;

    // Find a voice of lowest priority available
    unsigned int new_prio = VOICE_PRIO_NEW + 1;
    Voice* new_voice = NULL;
    for (uint16_t i = 0; i < pool->size; ++i)
    {
        Voice* voice = pool->voices[i];
        if ((voice->prio < new_prio) && (voice->group_id != group_id))
        {
            new_voice = voice;
            new_prio = voice->prio;
        }
    }

    rassert(new_voice != NULL);
    rassert(new_voice->group_id != group_id);

    if (new_voice->group_id != 0)
        Voice_pool_reset_group(pool, new_voice->group_id);

    // Pre-init the voice
    new_voice->prio = VOICE_PRIO_INACTIVE;
    new_voice->group_id = group_id;

    return new_voice;
}


static uint64_t get_voice_group_prio(const Voice* voice)
{
    // Overflow group ID 0 to maximum so that inactive voices are placed last
    return Voice_get_group_id(voice) - 1;
}


void Voice_pool_free_inactive(Voice_pool* pool)
{
    rassert(pool != NULL);

    for (uint16_t i = 0; i < pool->size; ++i)
    {
        Voice* current = pool->voices[i];
        if (current->prio == VOICE_PRIO_INACTIVE)
            Voice_reset(current);
    }

    return;
}


void Voice_pool_sort_groups(Voice_pool* pool)
{
    rassert(pool != NULL);

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


void Voice_pool_reset_group(Voice_pool* pool, uint64_t group_id)
{
    rassert(pool != NULL);
    rassert(group_id != 0);

    for (int i = 0; i < pool->size; ++i)
    {
        Voice* voice = pool->voices[i];
        if (voice->group_id == group_id)
            Voice_reset(voice);
    }

    return;
}


Voice_group* Voice_pool_get_group(
        const Voice_pool* pool, uint64_t group_id, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(group_id != 0);
    rassert(vgroup != NULL);

    for (int i = 0; i < pool->size; ++i)
    {
        if (pool->voices[i]->group_id == group_id)
        {
            Voice_group_init(vgroup, pool->voices, i, pool->size);
            return vgroup;
        }
    }

    return NULL;
}


void Voice_pool_start_group_iteration(Voice_pool* pool)
{
    rassert(pool != NULL);

    Voice_pool_sort_groups(pool);

    pool->group_iter_offset = 0;

    return;
}


Voice_group* Voice_pool_get_next_group(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    if (pool->group_iter_offset >= pool->size)
        return NULL;

    Voice_group_init(vgroup, pool->voices, pool->group_iter_offset, pool->size);
    pool->group_iter_offset += Voice_group_get_size(vgroup);

    if (Voice_group_get_size(vgroup) == 0)
        return NULL;

    return vgroup;
}


Voice_group* Voice_pool_get_next_fg_group(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    Voice_group* vg = Voice_pool_get_next_group(pool, vgroup);
    while ((vg != NULL) && Voice_group_is_bg(vg))
        vg = Voice_pool_get_next_group(pool, vgroup);

    return vg;
}


Voice_group* Voice_pool_get_next_bg_group(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    Voice_group* vg = Voice_pool_get_next_group(pool, vgroup);
    while ((vg != NULL) && !Voice_group_is_bg(vg))
        vg = Voice_pool_get_next_group(pool, vgroup);

    return vg;
}


#ifdef ENABLE_THREADS
Voice_group* Voice_pool_get_next_group_synced(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    Mutex_lock(&pool->group_iter_lock);

    if (pool->group_iter_offset >= pool->size)
    {
        Mutex_unlock(&pool->group_iter_lock);
        return NULL;
    }

    Voice_group_init(vgroup, pool->voices, pool->group_iter_offset, pool->size);
    pool->group_iter_offset += Voice_group_get_size(vgroup);

    const bool is_group_empty = (Voice_group_get_size(vgroup) == 0);

    Mutex_unlock(&pool->group_iter_lock);

    if (is_group_empty)
        return NULL;

    return vgroup;
}


Voice_group* Voice_pool_get_next_fg_group_synced(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    Mutex_lock(&pool->group_iter_lock);

    Voice_group* vg = Voice_pool_get_next_fg_group(pool, vgroup);

    Mutex_unlock(&pool->group_iter_lock);

    return vg;
}


Voice_group* Voice_pool_get_next_bg_group_synced(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    Mutex_lock(&pool->group_iter_lock);

    Voice_group* vg = Voice_pool_get_next_bg_group(pool, vgroup);

    Mutex_unlock(&pool->group_iter_lock);

    return vg;
}
#endif


void Voice_pool_reset(Voice_pool* pool)
{
    rassert(pool != NULL);

    for (uint16_t i = 0; i < pool->size; ++i)
        Voice_reset(pool->voices[i]);

    return;
}


void del_Voice_pool(Voice_pool* pool)
{
    if (pool == NULL)
        return;

    Mutex_deinit(&pool->group_iter_lock);

    if (pool->voices != NULL)
    {
        for (uint16_t i = 0; i < pool->size; ++i)
        {
            del_Voice(pool->voices[i]);
            pool->voices[i] = NULL;
        }
    }
    del_Voice_work_buffers(pool->voice_wbs);
    memory_free(pool->voices);
    memory_free(pool);

    return;
}


