

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string.h>

#include <debug/assert.h>
#include <memory.h>
#include <player/Work_buffer.h>


#define WORK_BUFFER_ELEM_SIZE (sizeof(float))

static_assert(sizeof(int32_t) <= WORK_BUFFER_ELEM_SIZE,
        "Work buffers must have space for enough 32-bit integers.");


struct Work_buffer
{
    uint32_t size;
    char* contents;
};


Work_buffer* new_Work_buffer(uint32_t size)
{
    //assert(size >= 0);
    assert(size <= WORK_BUFFER_SIZE_MAX);

    Work_buffer* buffer = memory_alloc_item(Work_buffer);
    if (buffer == NULL)
        return NULL;

    // Sanitise fields
    buffer->size = size;
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


bool Work_buffer_resize(Work_buffer* buffer, uint32_t new_size)
{
    assert(buffer != NULL);
    //assert(new_size >= 0);
    assert(new_size <= WORK_BUFFER_SIZE_MAX);

    if (new_size == 0)
    {
        buffer->size = new_size;
        memory_free(buffer->contents);
        buffer->contents = NULL;
        return true;
    }

    const uint32_t actual_size = new_size + 2;
    char* new_contents = memory_realloc_items(
            char, actual_size * WORK_BUFFER_ELEM_SIZE, buffer->contents);
    if (new_contents == NULL)
        return false;

    buffer->size = new_size;
    buffer->contents = new_contents;

    return true;
}


uint32_t Work_buffer_get_size(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return buffer->size;
}


const float* Work_buffer_get_contents(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return (float*)buffer->contents + 1;
}


float* Work_buffer_get_contents_mut(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return (float*)buffer->contents + 1;
}


int32_t* Work_buffer_get_contents_int_mut(const Work_buffer* buffer)
{
    assert(buffer != NULL);
    return (int32_t*)buffer->contents + 1;
}


void Work_buffer_copy(
        const Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        uint32_t start,
        uint32_t stop)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(dest != src);
    assert(start <= Work_buffer_get_size(dest));
    assert(stop <= Work_buffer_get_size(dest));

    if (start >= stop)
        return;

    char* dest_start = dest->contents + (start * WORK_BUFFER_ELEM_SIZE);
    const char* src_start = src->contents + (start * WORK_BUFFER_ELEM_SIZE);
    const uint32_t elem_count = stop - start;
    memcpy(dest_start, src_start, elem_count * WORK_BUFFER_ELEM_SIZE);

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


