

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <debug/assert.h>
#include <memory.h>
#include <player/Voice_pool.h>


Voice_pool* new_Voice_pool(uint16_t size)
{
    //assert(size >= 0);

    Voice_pool* pool = memory_alloc_item(Voice_pool);
    if (pool == NULL)
        return NULL;

    pool->size = size;
    pool->state_size = 0;
    pool->voices = NULL;

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


bool Voice_pool_reserve_state_space(Voice_pool* pool, size_t state_size)
{
    assert(pool != NULL);

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


bool Voice_pool_resize(Voice_pool* pool, uint16_t size)
{
    assert(pool != NULL);
    //assert(size >= 0);

    uint16_t new_size = size;
    if (new_size == pool->size)
        return true;

    // Remove excess voices if any
    for (uint16_t i = new_size; i < pool->size; ++i)
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
    for (uint16_t i = pool->size; i < new_size; ++i)
        pool->voices[i] = NULL;

    // Allocate new voices
    for (uint16_t i = pool->size; i < new_size; ++i)
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


uint16_t Voice_pool_get_size(Voice_pool* pool)
{
    assert(pool != NULL);
    return pool->size;
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


void Voice_pool_prepare(Voice_pool* pool)
{
    assert(pool != NULL);

    for (uint16_t i = 0; i < pool->size; ++i)
    {
        if (pool->voices[i]->prio != VOICE_PRIO_INACTIVE)
            Voice_prepare(pool->voices[i]);
    }

    return;
}


uint16_t Voice_pool_mix_bg(
        Voice_pool* pool,
        Device_states* states,
        uint32_t amount,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(pool != NULL);
    assert(states != NULL);
    assert(freq > 0);

    if (pool->size == 0)
        return 0;

    uint16_t active_voices = 0;
    for (uint16_t i = 0; i < pool->size; ++i)
    {
        if (pool->voices[i]->prio != VOICE_PRIO_INACTIVE)
        {
            if (pool->voices[i]->prio <= VOICE_PRIO_BG)
            {
//                fprintf(stderr, "Background mix start\n");
                Voice_mix(pool->voices[i], states, amount, offset, freq, tempo);
//                fprintf(stderr, "Background mix end\n");
            }
            ++active_voices;
        }
    }

    return active_voices;
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


