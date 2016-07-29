

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Voice_work_buffers.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffer.h>
#include <player/Work_buffer_private.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Voice_work_buffers
{
    int count;
    int32_t buf_size;
    Work_buffer* wbs;
    void* space;
};


Voice_work_buffers* new_Voice_work_buffers(void)
{
    Voice_work_buffers* wbs = memory_alloc_item(Voice_work_buffers);
    if (wbs == NULL)
        return NULL;

    wbs->count = 0;
    wbs->buf_size = 0;
    wbs->wbs = NULL;
    wbs->space = NULL;

    return wbs;
}


int32_t Voice_work_buffers_get_buffer_size(const Voice_work_buffers* wbs)
{
    rassert(wbs != NULL);
    return wbs->buf_size;
}


bool Voice_work_buffers_allocate_space(
        Voice_work_buffers* wbs, int count, int32_t buf_size)
{
    rassert(wbs != NULL);
    rassert(count >= 0);
    rassert(count <= KQT_VOICES_MAX);
    rassert(buf_size >= 0);
    rassert(buf_size <= VOICE_WORK_BUFFER_SIZE_MAX);

    if ((wbs->count == count) && (wbs->buf_size == buf_size))
        return true;

    if (count == 0 || buf_size == 0)
    {
        wbs->count = count;
        wbs->buf_size = buf_size;

        memory_free(wbs->wbs);
        wbs->wbs = NULL;
        memory_free(wbs->space);
        wbs->space = NULL;

        return true;
    }

    const int32_t actual_buf_size = buf_size + 2;

    // Allocate memory
    Work_buffer* new_wbs = memory_realloc_items(Work_buffer, count, wbs->wbs);
    if (new_wbs == NULL)
        return false;
    wbs->wbs = new_wbs;

    wbs->count = min(wbs->count, count);

    const int32_t total_space_size = count * actual_buf_size;
    void* new_space = memory_realloc_items(float, total_space_size, wbs->space);
    if (new_space == NULL)
        return false;
    wbs->space = new_space;

    wbs->count = count;
    wbs->buf_size = buf_size;

    // Initialise Work buffers
    for (int i = 0; i < wbs->count; ++i)
        Work_buffer_init_with_memory(
                &wbs->wbs[i],
                (float*)wbs->space + (i * actual_buf_size),
                actual_buf_size);

    return true;
}


Work_buffer* Voice_work_buffers_get_buffer_mut(const Voice_work_buffers* wbs, int index)
{
    rassert(wbs != NULL);
    rassert(index >= 0);
    rassert(index < wbs->count);

    if (wbs->wbs == NULL)
        return NULL;

    return &wbs->wbs[index];
}


void del_Voice_work_buffers(Voice_work_buffers* wbs)
{
    if (wbs == NULL)
        return;

    memory_free(wbs->wbs);
    memory_free(wbs->space);
    memory_free(wbs);

    return;
}


