

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
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


static_assert(sizeof(int32_t) <= WORK_BUFFER_ELEM_SIZE,
        "Work buffers must have space for enough 32-bit integers.");


#define MARGIN_ELEM_COUNT 3


Work_buffer* new_Work_buffer(int32_t size)
{
    rassert(size > 0);
    rassert(size <= WORK_BUFFER_SIZE_MAX);

    Work_buffer* buffer = memory_alloc_item(Work_buffer);
    if (buffer == NULL)
        return NULL;

    // Sanitise fields
    buffer->size = size;
    buffer->is_valid = true;
    buffer->const_start = 0;
    buffer->is_final = true;
    buffer->contents = NULL;

    // Allocate buffers
    const int32_t actual_size = size + MARGIN_ELEM_COUNT;
    buffer->contents =
        memory_alloc_items_aligned(char, actual_size * WORK_BUFFER_ELEM_SIZE, 64);
    if (buffer->contents == NULL)
    {
        del_Work_buffer(buffer);
        return NULL;
    }

    float* contents = buffer->contents;
    for (int32_t i = 0; i < actual_size; ++i)
        contents[i] = 0;

    return buffer;
}


void Work_buffer_init_with_memory(
        Work_buffer* buffer, void* space, int32_t raw_elem_count)
{
    rassert(buffer != NULL);
    rassert(space != NULL);
    rassert((intptr_t)space % 64 == 0);
    rassert(raw_elem_count >= 4);

    buffer->size = raw_elem_count - MARGIN_ELEM_COUNT;
    buffer->is_valid = true;
    buffer->const_start = 0;
    buffer->is_final = true;
    buffer->contents = space;

    Work_buffer_clear(buffer, 0, Work_buffer_get_size(buffer) + MARGIN_ELEM_COUNT);

    return;
}


void Work_buffer_invalidate(Work_buffer* buffer)
{
    rassert(buffer != NULL);

    buffer->is_valid = false;

#ifdef ENABLE_DEBUG_ASSERTS
    float* data = buffer->contents;
    for (int i = 0; i < buffer->size; ++i)
        *data++ = NAN;
#endif

    return;
}


void Work_buffer_mark_valid(Work_buffer* buffer)
{
    rassert(buffer != NULL);
    buffer->is_valid = true;
    return;
}


bool Work_buffer_is_valid(const Work_buffer* buffer)
{
    return (buffer != NULL) && buffer->is_valid;
}


bool Work_buffer_resize(Work_buffer* buffer, int32_t new_size)
{
    rassert(buffer != NULL);
    rassert(new_size > 0);
    rassert(new_size <= WORK_BUFFER_SIZE_MAX);

    const int32_t actual_size = new_size + MARGIN_ELEM_COUNT;
    char* new_contents =
        memory_alloc_items_aligned(char, actual_size * WORK_BUFFER_ELEM_SIZE, 64);
    if (new_contents == NULL)
        return false;

    memory_free_aligned(buffer->contents);
    buffer->contents = NULL;

    buffer->size = new_size;
    buffer->contents = new_contents;

    buffer->is_valid = false;
    Work_buffer_clear_const_start(buffer);
    buffer->is_final = false;

    return true;
}


void Work_buffer_clear(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop)
{
    rassert(buffer != NULL);
    rassert(buf_start >= 0);
    rassert(buf_start <= Work_buffer_get_size(buffer));
    rassert(buf_stop >= 0);
    rassert(buf_stop <= Work_buffer_get_size(buffer) + MARGIN_ELEM_COUNT);

    float* fcontents = Work_buffer_get_contents_mut(buffer) + buf_start;
    for (int32_t i = buf_start; i < buf_stop; ++i)
        *fcontents++ = 0;

    buffer->is_valid = true;
    Work_buffer_set_const_start(buffer, buf_start);
    buffer->is_final = true;

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
    rassert(Work_buffer_is_valid(buffer));

    return (float*)buffer->contents;
}


float* Work_buffer_get_contents_mut(Work_buffer* buffer)
{
    rassert(buffer != NULL);

    Work_buffer_mark_valid(buffer);
    Work_buffer_clear_const_start(buffer);
    Work_buffer_set_final(buffer, false);

    return (float*)buffer->contents;
}


int32_t* Work_buffer_get_contents_int_mut(Work_buffer* buffer)
{
    rassert(buffer != NULL);

    Work_buffer_mark_valid(buffer);
    Work_buffer_clear_const_start(buffer);
    Work_buffer_set_final(buffer, false);

    return (int32_t*)buffer->contents;
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
    rassert(buf_start >= 0);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= 0);
    rassert(buf_stop <= Work_buffer_get_size(dest) + MARGIN_ELEM_COUNT);

    if (buf_start >= buf_stop)
        return;

    if (!Work_buffer_is_valid(src))
        return;

    float* dest_pos = (float*)dest->contents + buf_start;
    const float* src_pos = (const float*)src->contents + buf_start;

    const int32_t elem_count = buf_stop - buf_start;
    for (int32_t i = 0; i < elem_count; ++i)
    {
        dassert(!isnan(*src_pos));
        *dest_pos++ = *src_pos++;
    }

    Work_buffer_mark_valid(dest);
    Work_buffer_set_const_start(dest, Work_buffer_get_const_start(src));
    Work_buffer_set_final(dest, Work_buffer_is_final(src));

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
        Work_buffer* dest, const Work_buffer* src, int32_t buf_start, int32_t buf_stop)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(Work_buffer_get_size(dest) == Work_buffer_get_size(src));
    rassert(buf_start >= 0);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= 0);
    rassert(buf_stop <= Work_buffer_get_size(dest) + MARGIN_ELEM_COUNT);

    if (dest == src)
        return;

    if (buf_start >= buf_stop)
        return;

    if (!Work_buffer_is_valid(src))
        return;

    if (!Work_buffer_is_valid(dest))
    {
        Work_buffer_copy(dest, src, buf_start, buf_stop);
        return;
    }

    const int32_t orig_const_start = Work_buffer_get_const_start(dest);
    const int32_t src_const_start = Work_buffer_get_const_start(src);

    const bool buffer_has_final_value =
        Work_buffer_is_final(dest) && (orig_const_start < buf_stop);
    const bool in_has_final_value =
        Work_buffer_is_final(src) && (src_const_start < buf_stop);

    float* dest_contents = (float*)dest->contents;
    const float* src_contents = Work_buffer_get_contents(src);

    const bool buffer_has_neg_inf_final_value =
        buffer_has_final_value && (dest_contents[orig_const_start] == -INFINITY);
    const bool in_has_neg_inf_final_value =
        in_has_final_value && (src_contents[src_const_start] == -INFINITY);

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        dassert(!isnan(dest_contents[i]));
        dassert(!isnan(src_contents[i]));
        dest_contents[i] += src_contents[i];
    }

    bool result_is_const_final = (buffer_has_final_value && in_has_final_value);
    int32_t new_const_start = max(orig_const_start, src_const_start);

    // Fill result buffer trail with negative infinity
    // if the input ends with final negative infinity
    if (buffer_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, orig_const_start);

        for (int32_t i = orig_const_start; i < buf_stop; ++i)
            dest_contents[i] = -INFINITY;
    }

    if (in_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, src_const_start);

        for (int32_t i = src_const_start; i < buf_stop; ++i)
            dest_contents[i] = -INFINITY;
    }

    Work_buffer_mark_valid(dest);
    Work_buffer_set_const_start(dest, new_const_start);
    Work_buffer_set_final(dest, result_is_const_final);

    return;
}


void Work_buffer_mix_shifted(
        Work_buffer* restrict dest,
        int32_t dest_offset,
        const Work_buffer* restrict src,
        int32_t item_count)
{
    rassert(dest != NULL);
    rassert(dest_offset >= 0);
    rassert(src != NULL);
    rassert(src != dest);
    rassert(item_count >= 0);
    rassert(item_count + dest_offset <= Work_buffer_get_size(dest));

    if (!Work_buffer_is_valid(src))
        return;

    const int32_t src_const_start = Work_buffer_get_const_start(src);
    const int32_t shifted_src_const_start = (src_const_start < INT32_MAX - dest_offset)
        ? src_const_start + dest_offset : src_const_start;

    float* dest_contents = (float*)dest->contents + dest_offset;
    const float* src_contents = Work_buffer_get_contents(src);

    if (!Work_buffer_is_valid(dest))
    {
        for (int32_t i = 0; i < item_count; ++i)
        {
            dassert(!isnan(*src_contents));
            *dest_contents++ = *src_contents++;
        }

        Work_buffer_mark_valid(dest);
        Work_buffer_set_const_start(dest, shifted_src_const_start);
        Work_buffer_set_final(dest, Work_buffer_is_final(src));

        return;
    }

    const int32_t dest_const_start = Work_buffer_get_const_start(dest);

    const bool dest_has_final_value =
        Work_buffer_is_final(dest) && (dest_const_start < item_count - dest_offset);
    const bool src_has_final_value =
        Work_buffer_is_final(src) && (src_const_start < item_count);

    // NOTE: dest_contents is already shifted
    const bool dest_has_neg_inf_final_value =
        dest_has_final_value && (dest_contents[dest_const_start] == -INFINITY);
    const bool src_has_neg_inf_final_value =
        src_has_final_value && (src_contents[src_const_start] == -INFINITY);

    for (int32_t i = 0; i < item_count; ++i)
    {
        dassert(!isnan(dest_contents[i]));
        dassert(!isnan(src_contents[i]));
        dest_contents[i] += src_contents[i];
    }

    bool result_is_const_final = (dest_has_final_value && src_has_final_value);
    int32_t new_const_start = max(dest_const_start, shifted_src_const_start);

    // Fill result buffer trail with negative infinity
    // if the input ends with final negative infinity
    if (dest_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, dest_const_start);

        for (int32_t i = dest_const_start; i < item_count; ++i)
            dest_contents[i] = -INFINITY;
    }

    if (src_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, shifted_src_const_start);

        for (int32_t i = src_const_start; i < item_count; ++i)
            dest_contents[i] = -INFINITY;
    }

    Work_buffer_mark_valid(dest);
    Work_buffer_set_const_start(dest, new_const_start);
    Work_buffer_set_final(dest, result_is_const_final);

    return;
}


void del_Work_buffer(Work_buffer* buffer)
{
    if (buffer == NULL)
        return;

    memory_free_aligned(buffer->contents);
    memory_free(buffer);

    return;
}


