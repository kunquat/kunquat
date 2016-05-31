

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Work_buffer.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define WORK_BUFFER_ELEM_SIZE (sizeof(float))

static_assert(sizeof(int32_t) <= WORK_BUFFER_ELEM_SIZE,
        "Work buffers must have space for enough 32-bit integers.");


struct Work_buffer
{
    int32_t size;
    int32_t const_start;
    char* contents;
};


Work_buffer* new_Work_buffer(int32_t size)
{
    assert(size >= 0);
    assert(size <= WORK_BUFFER_SIZE_MAX);

    Work_buffer* buffer = memory_alloc_item(Work_buffer);
    if (buffer == NULL)
        return NULL;

    // Sanitise fields
    buffer->size = size;
    buffer->const_start = INT32_MAX;
    buffer->contents = NULL;

    if (buffer->size > 0)
    {
        // Allocate buffers
        const uint32_t actual_size = size + 2;
        buffer->contents = memory_calloc_items(
                char, actual_size * WORK_BUFFER_ELEM_SIZE);
        if (buffer->contents == NULL)
        {
            del_Work_buffer(buffer);
            return NULL;
        }
    }

    return buffer;
}


bool Work_buffer_resize(Work_buffer* buffer, int32_t new_size)
{
    assert(buffer != NULL);
    assert(new_size >= 0);
    assert(new_size <= WORK_BUFFER_SIZE_MAX);

    if (new_size == 0)
    {
        buffer->size = new_size;
        memory_free(buffer->contents);
        buffer->contents = NULL;
        return true;
    }

    const int32_t actual_size = new_size + 2;
    char* new_contents = memory_realloc_items(
            char, actual_size * WORK_BUFFER_ELEM_SIZE, buffer->contents);
    if (new_contents == NULL)
        return false;

    buffer->size = new_size;
    buffer->contents = new_contents;

    Work_buffer_clear_const_start(buffer);

    return true;
}


void Work_buffer_clear(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop)
{
    assert(buffer != NULL);
    assert(buf_start >= -1);
    assert(buf_start <= Work_buffer_get_size(buffer));
    assert(buf_stop >= -1);
    assert(buf_stop <= Work_buffer_get_size(buffer) + 1);

    float* fcontents = Work_buffer_get_contents_mut(buffer);
    for (int32_t i = buf_start; i < buf_stop; ++i)
        fcontents[i] = 0;

    Work_buffer_set_const_start(buffer, buf_start);

    return;
}


int32_t Work_buffer_get_size(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return buffer->size;
}


const float* Work_buffer_get_contents(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return (float*)buffer->contents + 1;
}


float* Work_buffer_get_contents_mut(Work_buffer* buffer)
{
    assert(buffer != NULL);

    Work_buffer_clear_const_start(buffer);

    return (float*)buffer->contents + 1;
}


float* Work_buffer_get_contents_mut_keep_const(Work_buffer* buffer)
{
    assert(buffer != NULL);
    return (float*)buffer->contents + 1;
}


int32_t* Work_buffer_get_contents_int_mut(Work_buffer* buffer)
{
    assert(buffer != NULL);

    Work_buffer_clear_const_start(buffer);

    return (int32_t*)buffer->contents + 1;
}


void Work_buffer_copy(
        Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(dest != src);
    assert(buf_start >= -1);
    assert(buf_start <= Work_buffer_get_size(dest));
    assert(buf_stop >= -1);
    assert(buf_stop <= Work_buffer_get_size(dest) + 1);

    if (buf_start >= buf_stop)
        return;

    const int32_t actual_start = buf_start + 1;
    const int32_t actual_stop = buf_stop + 1;

    char* dest_start = dest->contents + (actual_start * WORK_BUFFER_ELEM_SIZE);
    const char* src_start = src->contents + (actual_start * WORK_BUFFER_ELEM_SIZE);
    const uint32_t elem_count = actual_stop - actual_start;
    memcpy(dest_start, src_start, elem_count * WORK_BUFFER_ELEM_SIZE);

    Work_buffer_set_const_start(dest, Work_buffer_get_const_start(src));

    return;
}


void Work_buffer_set_const_start(Work_buffer* buffer, int32_t start)
{
    assert(buffer != NULL);
    assert(start >= 0);

    buffer->const_start = start;

    return;
}


void Work_buffer_clear_const_start(Work_buffer* buffer)
{
    assert(buffer != NULL);
    buffer->const_start = INT32_MAX;
    return;
}


int32_t Work_buffer_get_const_start(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return buffer->const_start;
}


void Work_buffer_mix(
        Work_buffer* buffer,
        const Work_buffer* in,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(buffer != NULL);
    assert(in != NULL);
    assert(Work_buffer_get_size(buffer) == Work_buffer_get_size(in));
    assert(buf_start >= -1);
    assert(buf_start <= Work_buffer_get_size(buffer));
    assert(buf_stop >= -1);
    assert(buf_stop <= Work_buffer_get_size(buffer) + 1);

    if (buffer == in)
        return;

    float* buf_contents = Work_buffer_get_contents_mut_keep_const(buffer);
    const float* in_contents = Work_buffer_get_contents(in);

    for (int32_t i = buf_start; i < buf_stop; ++i)
        buf_contents[i] += in_contents[i];

    Work_buffer_set_const_start(buffer, max(buffer->const_start, in->const_start));

    return;
}


void del_Work_buffer(Work_buffer* buffer)
{
    if (buffer == NULL)
        return;

    memory_free(buffer->contents);
    memory_free(buffer);

    return;
}


