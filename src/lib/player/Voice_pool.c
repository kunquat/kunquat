

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
#include <kunquat/limits.h>
#include <memory.h>
#include <player/Voice_work_buffers.h>
#include <threads/Mutex.h>

#if ENABLE_THREADS
#include <stdatomic.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


struct Voice_pool
{
#if ENABLE_THREADS
    atomic_int_least16_t atomic_bg_iter_index;
#endif

    int size;
    int32_t state_size;
    uint64_t new_group_id;

    Voice voices[KQT_VOICES_MAX];

    int free_voice_count;
    Voice* free_voices[KQT_VOICES_MAX];

    Voice* foreground_voices[KQT_VOICES_MAX];
    Voice* background_voices[KQT_VOICES_MAX];

    struct
    {
        int start;
        int stop;
    } fg_iter_bounds[KQT_CHANNELS_MAX];

    int bg_iter_index;
    int bg_group_count;
    int16_t bg_group_offsets[KQT_VOICES_MAX];

    Voice_work_buffers* voice_wbs;
};


Voice_pool* new_Voice_pool(int size)
{
    rassert(size >= 0);

    Voice_pool* pool = memory_alloc_item(Voice_pool);
    if (pool == NULL)
        return NULL;

#if ENABLE_THREADS
    pool->atomic_bg_iter_index = -1;
#endif

    pool->size = size;
    pool->state_size = 0;
    pool->new_group_id = 0;
    pool->free_voice_count = 0;
    pool->bg_iter_index = 0;
    pool->bg_group_count = 0;
    pool->voice_wbs = NULL;

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        Voice_preinit(&pool->voices[i]);
        pool->free_voices[i] = NULL;
        pool->foreground_voices[i] = NULL;
        pool->background_voices[i] = NULL;
        pool->bg_group_offsets[i] = 0;
    }

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        pool->fg_iter_bounds[i].start = 0;
        pool->fg_iter_bounds[i].stop = 0;
    }

    pool->voice_wbs = new_Voice_work_buffers();
    if (pool->voice_wbs == NULL)
    {
        del_Voice_pool(pool);
        return NULL;
    }

    if (size > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            if (Voice_init(&pool->voices[i]) == NULL)
            {
                del_Voice_pool(pool);
                return NULL;
            }
        }

        for (int i = 0; i < size; ++i)
            pool->free_voices[i] = &pool->voices[i];

        pool->free_voice_count = size;
    }

    return pool;
}


bool Voice_pool_reserve_state_space(Voice_pool* pool, int32_t state_size)
{
    rassert(pool != NULL);
    rassert(state_size >= 0);

    if (state_size <= pool->state_size)
        return true;

    for (int i = 0; i < pool->size; ++i)
    {
        if (!Voice_reserve_state_space(&pool->voices[i], state_size))
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
                &pool->voices[i], Voice_work_buffers_get_buffer_mut(pool->voice_wbs, i));
    }

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


static Voice* try_extract_voice_from_array(Voice* voices[], uint64_t exclude_group_id)
{
    rassert(voices != NULL);
    rassert(exclude_group_id != 0);

    Voice* selected_voice = NULL;
    int selected_voice_index = 0;
    uint64_t selected_group_id = UINT64_MAX;

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        Voice* cur_voice = voices[i];
        if (cur_voice == NULL)
            break;

        if ((cur_voice->group_id != exclude_group_id) &&
                (cur_voice->group_id <= selected_group_id))
        {
            selected_voice = cur_voice;
            selected_voice_index = i;
            selected_group_id = cur_voice->group_id;
        }
    }

    if (selected_voice == NULL)
        return NULL;

    // Remove gap in the Voice array
    voices[selected_voice_index] = NULL;
    for (int i = selected_voice_index + 1; i < KQT_VOICES_MAX; ++i)
    {
        if (voices[i] == NULL)
            break;

        voices[i - 1] = voices[i];
        voices[i] = NULL;
    }

    return selected_voice;
}


Voice* Voice_pool_allocate_voice(
        Voice_pool* pool, int ch_num, uint64_t group_id, bool is_external)
{
    rassert(pool != NULL);
    rassert(pool->size > 0);
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(group_id != 0);

    Voice* new_voice = NULL;

    if (pool->free_voice_count > 0)
    {
        // Reserve a free voice
        int reserved_index = pool->free_voice_count - 1;
        new_voice = pool->free_voices[reserved_index];
        pool->free_voices[reserved_index] = NULL;
        --pool->free_voice_count;
    }
    else
    {
        // Try to find an old background voice
        Voice* bg_voice =
            try_extract_voice_from_array(pool->background_voices, group_id);

        if (bg_voice != NULL)
        {
            Voice_pool_reset_group(pool, bg_voice->group_id);
            new_voice = bg_voice;
        }
        else
        {
            // Get one of the oldest foreground voices as a last resort
            Voice* fg_voice =
                try_extract_voice_from_array(pool->foreground_voices, group_id);

            rassert(fg_voice != NULL);
            Voice_pool_reset_group(pool, fg_voice->group_id);
            new_voice = fg_voice;
        }
    }

    rassert(new_voice != NULL);
    rassert(new_voice->group_id != group_id);

    // Pre-init the voice
    Voice_reserve(new_voice, group_id, ch_num, is_external);

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        if (pool->foreground_voices[i] == NULL)
        {
            pool->foreground_voices[i] = new_voice;
            return new_voice;
        }
    }

    rassert(false); // Couldn't find a location for the new voice in foreground array

    return NULL;
}


static void reset_voices_in_array(Voice_pool* pool, Voice* voices[], uint64_t group_id)
{
    rassert(voices != NULL);
    rassert(group_id != 0);

    int read_pos = 0;
    int write_pos = 0;

    while (read_pos < KQT_VOICES_MAX)
    {
        if (write_pos < read_pos)
        {
            voices[write_pos] = voices[read_pos];
            voices[read_pos] = NULL;
        }

        Voice* cur_voice = voices[write_pos];
        if (cur_voice == NULL)
            break;

        if (cur_voice->group_id == group_id)
        {
            voices[write_pos] = NULL;

            Voice_reset(cur_voice);

            rassert(pool->free_voice_count < KQT_VOICES_MAX);
            pool->free_voices[pool->free_voice_count] = cur_voice;
            ++pool->free_voice_count;
        }
        else
        {
            ++write_pos;
        }

        ++read_pos;
    }

    return;
}


void Voice_pool_reset_group(Voice_pool* pool, uint64_t group_id)
{
    rassert(pool != NULL);
    rassert(group_id != 0);

    reset_voices_in_array(pool, pool->foreground_voices, group_id);
    reset_voices_in_array(pool, pool->background_voices, group_id);

    return;
}


Voice_group* Voice_pool_get_fg_group(
        Voice_pool* pool, int ch_num, uint64_t group_id, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(group_id != 0);
    rassert(vgroup != NULL);

    // TODO: Consider using the channel number as context

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        if (pool->foreground_voices[i] == NULL)
            break;

        if (pool->foreground_voices[i]->group_id == group_id)
        {
            Voice_group_init(vgroup, pool->foreground_voices, i, KQT_VOICES_MAX);
            return vgroup;
        }
    }

    return NULL;
}


static int cmp_fg_voice(const Voice* v1, const Voice* v2)
{
    dassert(v1 != NULL);
    dassert(v2 != NULL);
    dassert(v1 != v2);

    if (v1->ch_num < v2->ch_num)
        return -1;
    else if (v1->ch_num > v2->ch_num)
        return 1;

    if (v1->group_id < v2->group_id)
        return -1;
    else if (v1->group_id > v2->group_id)
        return 1;

    return 0;
}


static int cmp_bg_voice(const Voice* v1, const Voice* v2)
{
    dassert(v1 != NULL);
    dassert(v2 != NULL);
    dassert(v1 != v2);

    if (v1->group_id < v2->group_id)
        return -1;
    else if (v1->group_id > v2->group_id)
        return 1;

    return 0;
}


static void sort_voice_array(
        Voice* voices[], int (*cmp)(const Voice* v1, const Voice* v2))
{
    rassert(voices != NULL);
    rassert(cmp != NULL);

    for (uint16_t i = 1; i < KQT_VOICES_MAX; ++i)
    {
        Voice* cur_voice = voices[i];
        if (cur_voice == NULL)
            break;

        int target_index = i;
        for (; target_index > 0; --target_index)
        {
            Voice* prev_voice = voices[target_index - 1];
            if (cmp(prev_voice, cur_voice) <= 0)
                break;

            voices[target_index] = prev_voice;
        }

        voices[target_index] = cur_voice;
    }

    return;
}


void Voice_pool_sort_fg_groups(Voice_pool* pool)
{
    rassert(pool != NULL);

    sort_voice_array(pool->foreground_voices, cmp_fg_voice);

    return;
}


void Voice_pool_start_group_iteration(Voice_pool* pool)
{
    rassert(pool != NULL);

    sort_voice_array(pool->foreground_voices, cmp_fg_voice);
    sort_voice_array(pool->background_voices, cmp_bg_voice);

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        if (pool->foreground_voices[i] == NULL)
            break;
        pool->foreground_voices[i]->updated = false;
    }

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        if (pool->background_voices[i] == NULL)
            break;
        pool->background_voices[i]->updated = false;
    }

    // Initialise foreground iteration info
    {
        for (int ch = 0; ch < KQT_CHANNELS_MAX; ++ch)
        {
            pool->fg_iter_bounds[ch].start = 0;
            pool->fg_iter_bounds[ch].stop = 0;
        }

        int prev_ch_num = -1;
        for (int i = 0; i < KQT_VOICES_MAX; ++i)
        {
            const Voice* cur_voice = pool->foreground_voices[i];
            if (cur_voice == NULL)
            {
                if (prev_ch_num >= 0)
                    pool->fg_iter_bounds[prev_ch_num].stop = i;
                break;
            }

            const int cur_ch_num = cur_voice->ch_num;
            rassert(cur_ch_num >= 0);
            rassert(cur_ch_num >= prev_ch_num);

            if (cur_ch_num > prev_ch_num)
            {
                if (prev_ch_num >= 0)
                    pool->fg_iter_bounds[prev_ch_num].stop = i;

                pool->fg_iter_bounds[cur_ch_num].start = i;
            }

            prev_ch_num = cur_ch_num;
        }
    }

    // Initialise background iteration info
    {
#if ENABLE_THREADS
        pool->atomic_bg_iter_index = -1;
#endif
        pool->bg_iter_index = 0;
        pool->bg_group_count = 0;

        uint64_t prev_group_id = 0;

        for (int16_t i = 0; i < KQT_VOICES_MAX; ++i)
        {
            const Voice* cur_voice = pool->background_voices[i];
            if (cur_voice == NULL)
                break;

            if (cur_voice->group_id != prev_group_id)
            {
                pool->bg_group_offsets[pool->bg_group_count] = i;
                ++pool->bg_group_count;
                prev_group_id = cur_voice->group_id;
            }
        }
    }

    return;
}


void Voice_pool_start_fg_ch_iteration(const Voice_pool* pool, int ch_num, int* ch_iter)
{
    rassert(pool != NULL);
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(ch_iter != NULL);

    *ch_iter = pool->fg_iter_bounds[ch_num].start;

    return;
}


Voice_group* Voice_pool_get_next_fg_group(
        Voice_pool* pool, int ch_num, int* ch_iter, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(ch_iter != NULL);
    rassert(vgroup != NULL);

    const int stop_bound = pool->fg_iter_bounds[ch_num].stop;

    if (*ch_iter >= stop_bound)
        return NULL;

    Voice_group_init(vgroup, pool->foreground_voices, *ch_iter, stop_bound);
    if (Voice_group_get_size(vgroup) == 0)
        return NULL;

    *ch_iter += Voice_group_get_size(vgroup);

    return vgroup;
}


Voice_group* Voice_pool_get_next_bg_group(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    if (pool->bg_iter_index >= pool->bg_group_count)
        return NULL;

    const int iter_offset = pool->bg_group_offsets[pool->bg_iter_index];

    if ((iter_offset >= KQT_VOICES_MAX) ||
            (pool->background_voices[iter_offset] == NULL))
        return NULL;

    ++pool->bg_iter_index;

    Voice_group_init(vgroup, pool->background_voices, iter_offset, KQT_VOICES_MAX);

    return vgroup;
}


#if ENABLE_THREADS
Voice_group* Voice_pool_get_next_bg_group_synced(Voice_pool* pool, Voice_group* vgroup)
{
    rassert(pool != NULL);
    rassert(vgroup != NULL);

    bool success = false;
    int_least16_t iter_index = 0;
    int_least16_t old_iter_index = pool->atomic_bg_iter_index;
    for (int i = 0; i < pool->bg_group_count; ++i)
    {
        iter_index = (int_least16_t)(old_iter_index + 1);
        success = atomic_compare_exchange_strong(
                &pool->atomic_bg_iter_index, &old_iter_index, iter_index);
        if (success)
            break;
    }

    if (!success || (iter_index >= pool->bg_group_count))
        return NULL;

    const int iter_offset = pool->bg_group_offsets[iter_index];

    if ((iter_offset >= KQT_VOICES_MAX) ||
            (pool->background_voices[iter_offset] == NULL))
        return NULL;

    Voice_group_init(vgroup, pool->background_voices, iter_offset, KQT_VOICES_MAX);

    return vgroup;
}
#endif


#ifdef ENABLE_DEBUG_ASSERTS
static void Voice_pool_validate(const Voice_pool* pool)
{
    dassert(pool != NULL);

    // Verify that each work array is packed to the left
    {
        for (int i = 0; i < pool->free_voice_count; ++i)
            dassert(pool->free_voices[i] != NULL);
        for (int i = pool->free_voice_count; i < KQT_VOICES_MAX; ++i)
            dassert(pool->free_voices[i] == NULL);
    }

    {
        int fg_count = 0;
        while (pool->foreground_voices[fg_count] != NULL)
            ++fg_count;
        for (int i = fg_count; i < KQT_VOICES_MAX; ++i)
            dassert(pool->foreground_voices[i] == NULL);
    }

    {
        int bg_count = 0;
        while (pool->background_voices[bg_count] != NULL)
            ++bg_count;
        for (int i = bg_count; i < KQT_VOICES_MAX; ++i)
            dassert(pool->background_voices[i] == NULL);
    }

    return;
}
#endif


void Voice_pool_clean_up_fg_voices(Voice_pool* pool)
{
    rassert(pool != NULL);

    int background_end = 0;
    while ((background_end < KQT_VOICES_MAX) &&
            (pool->background_voices[background_end] != NULL))
        ++background_end;

    int read_pos = 0;
    int write_pos = 0;

    while (read_pos < KQT_VOICES_MAX)
    {
        if (write_pos < read_pos)
        {
            pool->foreground_voices[write_pos] = pool->foreground_voices[read_pos];
            pool->foreground_voices[read_pos] = NULL;
        }

        Voice* cur_voice = pool->foreground_voices[write_pos];
        if (cur_voice == NULL)
            break;

        if (cur_voice->prio == VOICE_PRIO_INACTIVE)
        {
            pool->foreground_voices[write_pos] = NULL;

            rassert(pool->free_voice_count < KQT_VOICES_MAX);
            pool->free_voices[pool->free_voice_count] = cur_voice;
            ++pool->free_voice_count;
        }
        else if (cur_voice->prio < VOICE_PRIO_FG)
        {
            pool->foreground_voices[write_pos] = NULL;

            rassert(background_end < KQT_VOICES_MAX);
            pool->background_voices[background_end] = cur_voice;
            ++background_end;
        }
        else
        {
            ++write_pos;
        }

        ++read_pos;
    }

#ifdef ENABLE_DEBUG_ASSERTS
    Voice_pool_validate(pool);
#endif

    return;
}


void Voice_pool_finish_group_iteration(Voice_pool* pool)
{
    rassert(pool != NULL);

    int background_end = 0;

    // Clean up background array
    {
        int read_pos = 0;
        int write_pos = 0;

        while (read_pos < KQT_VOICES_MAX)
        {
            if (write_pos < read_pos)
            {
                pool->background_voices[write_pos] = pool->background_voices[read_pos];
                pool->background_voices[read_pos] = NULL;
            }

            Voice* cur_voice = pool->background_voices[write_pos];
            if (cur_voice == NULL)
                break;

            if (!cur_voice->updated || (cur_voice->prio == VOICE_PRIO_INACTIVE))
            {
                pool->background_voices[write_pos] = NULL;

                rassert(pool->free_voice_count < KQT_VOICES_MAX);
                pool->free_voices[pool->free_voice_count] = cur_voice;
                ++pool->free_voice_count;
            }
            else
            {
                ++write_pos;
            }

            ++read_pos;
        }

        background_end = write_pos;
        rassert((background_end == KQT_VOICES_MAX) ||
                (pool->background_voices[background_end] == NULL));
    }

    // Clean up foreground array
    {
        int read_pos = 0;
        int write_pos = 0;

        while (read_pos < KQT_VOICES_MAX)
        {
            if (write_pos < read_pos)
            {
                pool->foreground_voices[write_pos] = pool->foreground_voices[read_pos];
                pool->foreground_voices[read_pos] = NULL;
            }

            Voice* cur_voice = pool->foreground_voices[write_pos];
            if (cur_voice == NULL)
                break;

            if (((cur_voice->prio <= VOICE_PRIO_FG) && !cur_voice->updated) ||
                    (cur_voice->prio == VOICE_PRIO_INACTIVE))
            {
                pool->foreground_voices[write_pos] = NULL;

                rassert(pool->free_voice_count < KQT_VOICES_MAX);
                pool->free_voices[pool->free_voice_count] = cur_voice;
                ++pool->free_voice_count;
            }
            else if (cur_voice->prio < VOICE_PRIO_FG)
            {
                pool->foreground_voices[write_pos] = NULL;

                rassert(background_end < KQT_VOICES_MAX);
                pool->background_voices[background_end] = cur_voice;
                ++background_end;
            }
            else
            {
                cur_voice->prio = VOICE_PRIO_FG;
                ++write_pos;
            }

            ++read_pos;
        }
    }

#if ENABLE_THREADS
    pool->atomic_bg_iter_index = -1;
#endif
    pool->bg_iter_index = 0;
    pool->bg_group_count = 0;

#ifdef ENABLE_DEBUG_ASSERTS
    Voice_pool_validate(pool);
#endif

    return;
}


void Voice_pool_reset(Voice_pool* pool)
{
    rassert(pool != NULL);

    for (int i = 0; i < pool->size; ++i)
        Voice_reset(&pool->voices[i]);

    for (int i = 0; i < pool->size; ++i)
        pool->free_voices[i] = &pool->voices[i];
    pool->free_voice_count = pool->size;

    for (int i = 0; i < KQT_VOICES_MAX; ++i)
    {
        pool->foreground_voices[i] = NULL;
        pool->background_voices[i] = NULL;
    }

    pool->bg_group_count = 0;

    return;
}


void del_Voice_pool(Voice_pool* pool)
{
    if (pool == NULL)
        return;

    for (int i = 0; i < pool->size; ++i)
        Voice_deinit(&pool->voices[i]);

    del_Voice_work_buffers(pool->voice_wbs);
    memory_free(pool);

    return;
}


