

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Work_buffers.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Work_buffers
{
    Work_buffer* buffers[WORK_BUFFER_COUNT_];
};


Work_buffers* new_Work_buffers(int32_t buf_size)
{
    rassert(buf_size >= 0);
    rassert(buf_size <= WORK_BUFFER_SIZE_MAX);

    Work_buffers* buffers = memory_alloc_item(Work_buffers);
    if (buffers == NULL)
        return NULL;

    // Sanitise fields
    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
        buffers->buffers[i] = NULL;

    // Allocate buffers
    if (buf_size > 0)
    {
        for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
        {
            buffers->buffers[i] = new_Work_buffer(buf_size, WORK_BUFFER_SUB_COUNT_MAX);
            if (buffers->buffers[i] == NULL)
            {
                del_Work_buffers(buffers);
                return NULL;
            }
        }
    }

    return buffers;
}


bool Work_buffers_resize(Work_buffers* buffers, int32_t new_size)
{
    rassert(buffers != NULL);
    rassert(new_size >= 0);
    rassert(new_size <= WORK_BUFFER_SIZE_MAX);

    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
    {
        if (buffers->buffers[i] == NULL)
        {
            buffers->buffers[i] = new_Work_buffer(new_size, WORK_BUFFER_SUB_COUNT_MAX);
            if (buffers->buffers[i] == NULL)
                return false;
        }
        else
        {
            if (!Work_buffer_resize(buffers->buffers[i], new_size))
                return false;
        }
    }

    return true;
}


Work_buffer* Work_buffers_get_buffer_mut(
        const Work_buffers* buffers, Work_buffer_type type, int sub_count)
{
    rassert(buffers != NULL);
    rassert(type < WORK_BUFFER_COUNT_);
    rassert(sub_count >= 1);
    rassert(sub_count <= WORK_BUFFER_SUB_COUNT_MAX);
    rassert(is_p2(sub_count));

    Work_buffer* buffer = buffers->buffers[type];
    rassert(buffer != NULL);
    Work_buffer_set_sub_count(buffer, sub_count);

    return buffer;
}


float* Work_buffers_get_buffer_contents_mut(
        const Work_buffers* buffers, Work_buffer_type type)
{
    rassert(buffers != NULL);
    rassert(type < WORK_BUFFER_COUNT_);

    Work_buffer* buffer = Work_buffers_get_buffer_mut(buffers, type, 1);
    rassert(buffer != NULL);

    return Work_buffer_get_contents_mut(buffer, 0);
}


int32_t* Work_buffers_get_buffer_contents_int_mut(
        const Work_buffers* buffers, Work_buffer_type type)
{
    rassert(buffers != NULL);
    rassert(type < WORK_BUFFER_COUNT_);

    rassert(buffers->buffers[type] != NULL);

    return Work_buffer_get_contents_int_mut(buffers->buffers[type], 0);
}


void del_Work_buffers(Work_buffers* buffers)
{
    if (buffers == NULL)
        return;

    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
        del_Work_buffer(buffers->buffers[i]);

    memory_free(buffers);

    return;
}


