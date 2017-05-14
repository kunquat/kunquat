

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
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
#include <player/Work_buffer_private.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define WORK_BUFFER_ELEM_SIZE ((int)sizeof(float))

static_assert(sizeof(int32_t) <= WORK_BUFFER_ELEM_SIZE,
        "Work buffers must have space for enough 32-bit integers.");


Work_buffer* new_Work_buffer(int32_t size)
{
    rassert(size >= 0);
    rassert(size <= WORK_BUFFER_SIZE_MAX);

    Work_buffer* buffer = memory_alloc_item(Work_buffer);
    if (buffer == NULL)
        return NULL;

    // Sanitise fields
    buffer->size = size;
    buffer->const_start = 0;
    buffer->is_final = true;
    buffer->is_unbounded = false;
    buffer->contents = NULL;

    if (buffer->size > 0)
    {
        // Allocate buffers
        const int32_t actual_size = size + 2;
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


Work_buffer* new_Work_buffer_unbounded(int32_t size)
{
    rassert(size >= 0);
    rassert(size <= INT32_MAX / WORK_BUFFER_ELEM_SIZE);

    Work_buffer* buffer = new_Work_buffer(0);
    if (buffer == NULL)
        return NULL;

    buffer->is_unbounded = true;
    if (!Work_buffer_resize(buffer, size))
    {
        del_Work_buffer(buffer);
        return NULL;
    }

    return buffer;
}


void Work_buffer_init_with_memory(
        Work_buffer* buffer, void* space, int32_t raw_elem_count)
{
    rassert(buffer != NULL);
    rassert(space != NULL);
    rassert(raw_elem_count >= 2);

    buffer->size = raw_elem_count - 2;
    buffer->const_start = 0;
    buffer->is_final = true;
    buffer->is_unbounded = false;
    buffer->contents = space;

    Work_buffer_clear(buffer, -1, Work_buffer_get_size(buffer) + 1);

    return;
}


bool Work_buffer_resize(Work_buffer* buffer, int32_t new_size)
{
    rassert(buffer != NULL);
    rassert(new_size >= 0);

    const int32_t max_size = buffer->is_unbounded
        ? (INT32_MAX / WORK_BUFFER_ELEM_SIZE) : WORK_BUFFER_SIZE_MAX;
    rassert(new_size <= max_size);

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
    Work_buffer_set_final(buffer, false);

    return true;
}


void Work_buffer_clear(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop)
{
    rassert(buffer != NULL);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(buffer));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(buffer) + 1);

    float* fcontents = Work_buffer_get_contents_mut(buffer);
    for (int32_t i = buf_start; i < buf_stop; ++i)
        fcontents[i] = 0;

    Work_buffer_set_const_start(buffer, max(0, buf_start));
    Work_buffer_set_final(buffer, true);

    return;
}


int32_t Work_buffer_get_size(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->size;
}


const float* Work_buffer_get_contents(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return (float*)buffer->contents + 1;
}


float* Work_buffer_get_contents_mut(Work_buffer* buffer)
{
    rassert(buffer != NULL);

    Work_buffer_clear_const_start(buffer);
    Work_buffer_set_final(buffer, false);

    return (float*)buffer->contents + 1;
}


int32_t* Work_buffer_get_contents_int_mut(Work_buffer* buffer)
{
    rassert(buffer != NULL);

    Work_buffer_clear_const_start(buffer);
    Work_buffer_set_final(buffer, false);

    return (int32_t*)buffer->contents + 1;
}


void Work_buffer_copy(
        Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(dest != src);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(dest) + 1);

    if (buf_start >= buf_stop)
        return;

    const int32_t actual_start = buf_start + 1;
    const int32_t actual_stop = buf_stop + 1;

    char* dest_start = (char*)dest->contents + (actual_start * WORK_BUFFER_ELEM_SIZE);
    const char* src_start =
        (char*)src->contents + (actual_start * WORK_BUFFER_ELEM_SIZE);
    const int32_t elem_count = actual_stop - actual_start;
    rassert(elem_count >= 0);
    memcpy(dest_start, src_start, (size_t)(elem_count * WORK_BUFFER_ELEM_SIZE));

    Work_buffer_set_const_start(dest, Work_buffer_get_const_start(src));

    return;
}


void Work_buffer_set_const_start(Work_buffer* buffer, int32_t start)
{
    rassert(buffer != NULL);
    rassert(start >= 0);

    buffer->const_start = start;

    return;
}


void Work_buffer_clear_const_start(Work_buffer* buffer)
{
    rassert(buffer != NULL);
    buffer->const_start = INT32_MAX;
    return;
}


int32_t Work_buffer_get_const_start(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->const_start;
}


void Work_buffer_set_final(Work_buffer* buffer, bool is_final)
{
    rassert(buffer != NULL);
    buffer->is_final = is_final;
    return;
}


bool Work_buffer_is_final(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->is_final;
}


void Work_buffer_mix(
        Work_buffer* buffer,
        const Work_buffer* in,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(buffer != NULL);
    rassert(in != NULL);
    rassert(Work_buffer_get_size(buffer) == Work_buffer_get_size(in));
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(buffer));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(buffer) + 1);

    if (buffer == in)
        return;

    const int32_t orig_const_start = Work_buffer_get_const_start(buffer);

    const bool buffer_has_final_value =
        (Work_buffer_is_final(buffer) && (orig_const_start < buf_stop));
    const bool in_has_final_value =
        (Work_buffer_is_final(in) && (in->const_start < buf_stop));

    float* buf_contents = Work_buffer_get_contents_mut(buffer);
    const float* in_contents = Work_buffer_get_contents(in);

    const bool buffer_has_neg_inf_final_value =
        (buffer_has_final_value && (buf_contents[orig_const_start] == -INFINITY));
    const bool in_has_neg_inf_final_value =
        (in_has_final_value && (in_contents[in->const_start] == -INFINITY));

    for (int32_t i = buf_start; i < buf_stop; ++i)
        buf_contents[i] += in_contents[i];

    bool result_is_const_final = (buffer_has_final_value && in_has_final_value);
    int32_t new_const_start = max(orig_const_start, in->const_start);

    // Fill result buffer trail with negative infinity
    // if one of the inputs ends with final negative infinity
    if (buffer_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, orig_const_start);

        for (int32_t i = orig_const_start; i < buf_stop; ++i)
            buf_contents[i] = -INFINITY;
    }

    if (in_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, in->const_start);

        for (int32_t i = in->const_start; i < buf_stop; ++i)
            buf_contents[i] = -INFINITY;
    }

    Work_buffer_set_const_start(buffer, new_const_start);
    Work_buffer_set_final(buffer, result_is_const_final);

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


