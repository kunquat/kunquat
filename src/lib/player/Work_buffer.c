

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


Work_buffer* new_Work_buffer(int32_t size, int sub_count)
{
    rassert(size >= 0);
    rassert(size <= WORK_BUFFER_SIZE_MAX);
    rassert(sub_count >= 1);
    rassert(sub_count <= WORK_BUFFER_SUB_COUNT_MAX);
    rassert(is_p2(sub_count));

    Work_buffer* buffer = memory_alloc_item(Work_buffer);
    if (buffer == NULL)
        return NULL;

    // Sanitise fields
    buffer->size = size;
    buffer->init_sub_count = sub_count;
    buffer->sub_count = sub_count;
    for (int i = 0; i < WORK_BUFFER_SUB_COUNT_MAX; ++i)
    {
        buffer->is_valid[i] = true;
        buffer->const_start[i] = 0;
        buffer->is_final[i] = true;
    }
    buffer->contents = NULL;

    if (buffer->size > 0)
    {
        // Allocate buffers
        const int32_t actual_size = size + 2;
        buffer->contents = memory_calloc_items(
                char, actual_size * sub_count * WORK_BUFFER_ELEM_SIZE);
        if (buffer->contents == NULL)
        {
            del_Work_buffer(buffer);
            return NULL;
        }
    }

    return buffer;
}


void Work_buffer_init_with_memory(
        Work_buffer* buffer, int sub_count, void* space, int32_t raw_elem_count)
{
    rassert(buffer != NULL);
    rassert(sub_count >= 1);
    rassert(sub_count <= WORK_BUFFER_SUB_COUNT_MAX);
    rassert(is_p2(sub_count));
    rassert(space != NULL);
    rassert(raw_elem_count >= sub_count * 2);
    rassert(raw_elem_count % sub_count == 0);

    buffer->size = (raw_elem_count / sub_count) - 2;
    buffer->init_sub_count = sub_count;
    buffer->sub_count = sub_count;
    for (int i = 0; i < WORK_BUFFER_SUB_COUNT_MAX; ++i)
    {
        buffer->is_valid[i] = true;
        buffer->const_start[i] = 0;
        buffer->is_final[i] = true;
    }
    buffer->contents = space;

    for (int sub_index = 0; sub_index < sub_count; ++sub_index)
        Work_buffer_clear(buffer, sub_index, -1, Work_buffer_get_size(buffer) + 1);

    return;
}


void Work_buffer_invalidate(Work_buffer* buffer)
{
    rassert(buffer != NULL);

    for (int i = 0; i < WORK_BUFFER_SUB_COUNT_MAX; ++i)
        buffer->is_valid[i] = false;

#if ENABLE_DEBUG_ASSERTS
    float* data = buffer->contents;
    for (int i = -buffer->sub_count; i < (buffer->size + 1) * buffer->sub_count; ++i)
        *data++ = NAN;
#endif

    return;
}


bool Work_buffer_is_valid(const Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    return buffer->is_valid[sub_index];
}


bool Work_buffer_resize(Work_buffer* buffer, int32_t new_size)
{
    rassert(buffer != NULL);
    rassert(new_size >= 0);
    rassert(new_size <= WORK_BUFFER_SIZE_MAX);

    if (new_size == 0)
    {
        buffer->size = new_size;
        memory_free(buffer->contents);
        buffer->contents = NULL;
        return true;
    }

    const int32_t actual_size = new_size + 2;
    char* new_contents = memory_realloc_items(
            char,
            actual_size * buffer->sub_count * WORK_BUFFER_ELEM_SIZE,
            buffer->contents);
    if (new_contents == NULL)
        return false;

    buffer->size = new_size;
    buffer->contents = new_contents;

    for (int sub_index = 0; sub_index < buffer->sub_count; ++sub_index)
    {
        buffer->is_valid[sub_index] = false;
        Work_buffer_clear_const_start(buffer, sub_index);
        Work_buffer_set_final(buffer, sub_index, false);
    }

    return true;
}


void Work_buffer_set_sub_count(Work_buffer* buffer, int sub_count)
{
    rassert(buffer != NULL);
    rassert(sub_count >= 1);
    rassert(sub_count <= buffer->init_sub_count);
    rassert(is_p2(sub_count));

    if (buffer->sub_count == sub_count)
        return;

    buffer->sub_count = sub_count;

    for (int i = 0; i < sub_count; ++i)
    {
        buffer->is_valid[i] = false;
        buffer->const_start[i] = INT32_MAX;
        buffer->is_final[i] = false;
    }

    return;
}


int Work_buffer_get_sub_count(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->sub_count;
}


int Work_buffer_get_stride(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->sub_count;
}


void Work_buffer_clear(
        Work_buffer* buffer, int sub_index, int32_t buf_start, int32_t buf_stop)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(buffer));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(buffer) + 1);

    float* fcontents = Work_buffer_get_contents_mut(buffer, sub_index);
    for (int32_t i = buf_start; i < buf_stop; ++i)
        fcontents[i * buffer->sub_count] = 0;

    buffer->is_valid[sub_index] = true;
    Work_buffer_set_const_start(buffer, sub_index, max(0, buf_start));
    Work_buffer_set_final(buffer, sub_index, true);

    return;
}


void Work_buffer_clear_all(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop)
{
    rassert(buffer != NULL);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(buffer));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(buffer) + 1);

    const int32_t actual_start = buf_start * buffer->sub_count;
    const int32_t actual_stop = buf_stop * buffer->sub_count;

    float* fcontents = Work_buffer_get_contents_mut(buffer, 0);
    for (int32_t i = actual_start; i < actual_stop; ++i)
        fcontents[i] = 0;

    for (int i = 0; i < buffer->sub_count; ++i)
    {
        buffer->is_valid[i] = true;
        Work_buffer_set_const_start(buffer, i, max(0, buf_start));
        Work_buffer_set_final(buffer, i, true);
    }

    return;
}


int32_t Work_buffer_get_size(const Work_buffer* buffer)
{
    rassert(buffer != NULL);
    return buffer->size;
}


const float* Work_buffer_get_contents(const Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    rassert(buffer->is_valid[sub_index]);

    return (float*)buffer->contents + buffer->sub_count + sub_index;
}


float* Work_buffer_get_contents_mut(Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    buffer->is_valid[sub_index] = true;
    Work_buffer_clear_const_start(buffer, sub_index);
    Work_buffer_set_final(buffer, sub_index, false);

    return (float*)buffer->contents + buffer->sub_count + sub_index;
}


int32_t* Work_buffer_get_contents_int_mut(Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);

    buffer->is_valid[sub_index] = true;
    Work_buffer_clear_const_start(buffer, sub_index);
    Work_buffer_set_final(buffer, sub_index, false);

    return (int32_t*)buffer->contents + buffer->sub_count + sub_index;
}


void Work_buffer_copy(
        Work_buffer* restrict dest,
        int dest_sub_index,
        const Work_buffer* restrict src,
        int src_sub_index,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(dest != NULL);
    rassert(dest_sub_index >= 0);
    rassert(dest_sub_index < dest->sub_count);
    rassert(src != NULL);
    rassert(dest != src);
    rassert(src_sub_index >= 0);
    rassert(src_sub_index < src->sub_count);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(dest) + 1);

    if (buf_start >= buf_stop)
        return;

    if (!src->is_valid[src_sub_index])
        return;

    const int32_t actual_start = buf_start + 1;
    const int32_t actual_stop = buf_stop + 1;

    float* dest_pos =
        (float*)dest->contents + (actual_start * dest->sub_count) + dest_sub_index;
    const int dest_stride = Work_buffer_get_stride(dest);

    const float* src_pos =
        (const float*)src->contents + (actual_start * src->sub_count) + src_sub_index;
    const int src_stride = Work_buffer_get_stride(src);

    const int32_t elem_count = actual_stop - actual_start;
    for (int32_t i = 0; i < elem_count; ++i)
    {
        dassert(!isnan(*src_pos));
        *dest_pos = *src_pos;
        dest_pos += dest_stride;
        src_pos += src_stride;
    }

    dest->is_valid[dest_sub_index] = true;
    Work_buffer_set_const_start(
            dest, dest_sub_index, Work_buffer_get_const_start(src, src_sub_index));
    Work_buffer_set_final(
            dest, dest_sub_index, Work_buffer_is_final(src, src_sub_index));

    return;
}


void Work_buffer_copy_all(
        Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(dest != src);
    rassert(dest->sub_count == src->sub_count);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(dest) + 1);

    if (buf_start >= buf_stop)
        return;

    {
        bool is_src_fully_valid = true;
        for (int i = 0; i < src->sub_count; ++i)
            is_src_fully_valid &= src->is_valid[i];

        if (!is_src_fully_valid)
        {
            for (int i = 0; i < src->sub_count; ++i)
                Work_buffer_copy(dest, i, src, i, buf_start, buf_stop);

            return;
        }
    }

    const int32_t actual_start = buf_start + 1;
    const int32_t actual_stop = buf_stop + 1;
    float* dest_pos = (float*)dest->contents + (actual_start * dest->sub_count);
    const float* src_pos = (const float*)src->contents + (actual_start * dest->sub_count);
    for (int32_t i = actual_start; i < actual_stop; ++i)
    {
        dassert(!isnan(*src_pos));
        *dest_pos++ = *src_pos++;
    }

    for (int i = 0; i < dest->sub_count; ++i)
    {
        dest->is_valid[i] = true;
        Work_buffer_set_const_start(dest, i, Work_buffer_get_const_start(src, i));
        Work_buffer_set_final(dest, i, Work_buffer_is_final(src, i));
    }

    return;
}


void Work_buffer_set_const_start(Work_buffer* buffer, int sub_index, int32_t start)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);
    rassert(start >= 0);

    buffer->const_start[sub_index] = start;

    return;
}


void Work_buffer_clear_const_start(Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    buffer->const_start[sub_index] = INT32_MAX;

    return;
}


int32_t Work_buffer_get_const_start(const Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    return buffer->const_start[sub_index];
}


void Work_buffer_set_final(Work_buffer* buffer, int sub_index, bool is_final)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    buffer->is_final[sub_index] = is_final;

    return;
}


bool Work_buffer_is_final(const Work_buffer* buffer, int sub_index)
{
    rassert(buffer != NULL);
    rassert(sub_index >= 0);
    rassert(sub_index < buffer->sub_count);

    return buffer->is_final[sub_index];
}


void Work_buffer_mix(
        Work_buffer* dest,
        int dest_sub_index,
        const Work_buffer* src,
        int src_sub_index,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(dest != NULL);
    rassert(dest_sub_index >= 0);
    rassert(dest_sub_index < dest->sub_count);
    rassert(src != NULL);
    rassert(src_sub_index >= 0);
    rassert(src_sub_index < src->sub_count);
    rassert(Work_buffer_get_size(dest) == Work_buffer_get_size(src));
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(dest) + 1);

    if (dest == src)
        return;

    if (!src->is_valid[src_sub_index])
        return;

    if (!dest->is_valid[dest_sub_index])
    {
        Work_buffer_copy(dest, dest_sub_index, src, src_sub_index, buf_start, buf_stop);
        return;
    }

    const int32_t orig_const_start = Work_buffer_get_const_start(dest, dest_sub_index);
    const int32_t src_const_start = Work_buffer_get_const_start(src, src_sub_index);

    const bool buffer_has_final_value =
        (Work_buffer_is_final(dest, dest_sub_index) && (orig_const_start < buf_stop));
    const bool in_has_final_value =
        (Work_buffer_is_final(src, src_sub_index) && (src_const_start < buf_stop));

    float* dest_contents = Work_buffer_get_contents_mut(dest, dest_sub_index);
    const float* src_contents = Work_buffer_get_contents(src, src_sub_index);

    const bool buffer_has_neg_inf_final_value =
        (buffer_has_final_value &&
         (dest_contents[orig_const_start * dest->sub_count] == -INFINITY));
    const bool in_has_neg_inf_final_value =
        (in_has_final_value &&
         (src_contents[src_const_start * src->sub_count] == -INFINITY));

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        dassert(!isnan(dest_contents[i * dest->sub_count]));
        dassert(!isnan(src_contents[i * src->sub_count]));
        dest_contents[i * dest->sub_count] += src_contents[i * src->sub_count];
    }

    bool result_is_const_final = (buffer_has_final_value && in_has_final_value);
    int32_t new_const_start = max(orig_const_start, src_const_start);

    // Fill result buffer trail with negative infinity
    // if one of the inputs ends with final negative infinity
    if (buffer_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, orig_const_start);

        for (int32_t i = orig_const_start; i < buf_stop; ++i)
            dest_contents[i * dest->sub_count] = -INFINITY;
    }

    if (in_has_neg_inf_final_value)
    {
        result_is_const_final = true;
        new_const_start = min(new_const_start, src_const_start);

        for (int32_t i = src_const_start; i < buf_stop; ++i)
            dest_contents[i * dest->sub_count] = -INFINITY;
    }

    dest->is_valid[dest_sub_index] = true;
    Work_buffer_set_const_start(dest, dest_sub_index, new_const_start);
    Work_buffer_set_final(dest, dest_sub_index, result_is_const_final);

    return;
}


void Work_buffer_mix_all(
        Work_buffer* dest,
        const Work_buffer* src,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(Work_buffer_get_size(dest) == Work_buffer_get_size(src));
    rassert(dest->sub_count == src->sub_count);
    rassert(buf_start >= -1);
    rassert(buf_start <= Work_buffer_get_size(dest));
    rassert(buf_stop >= -1);
    rassert(buf_stop <= Work_buffer_get_size(dest) + 1);

    if (dest == src)
        return;

    {
        bool is_dest_fully_valid = true;
        bool is_src_fully_valid = true;
        for (int i = 0; i < dest->sub_count; ++i)
        {
            is_dest_fully_valid &= dest->is_valid[i];
            is_src_fully_valid &= src->is_valid[i];
        }

        if (!is_dest_fully_valid || !is_src_fully_valid)
        {
            for (int i = 0; i < dest->sub_count; ++i)
                Work_buffer_mix(dest, i, src, i, buf_start, buf_stop);

            return;
        }
    }

    int32_t dest_const_starts[WORK_BUFFER_SUB_COUNT_MAX] = { 0 };
    int32_t src_const_starts[WORK_BUFFER_SUB_COUNT_MAX] = { 0 };
    bool dest_has_final_value_at[WORK_BUFFER_SUB_COUNT_MAX] = { false };
    bool src_has_final_value_at[WORK_BUFFER_SUB_COUNT_MAX] = { false };

    for (int i = 0; i < dest->sub_count; ++i)
    {
        dest_const_starts[i] = dest->const_start[i];
        dest_has_final_value_at[i] =
            (Work_buffer_is_final(dest, i) && (dest_const_starts[i] < buf_stop));

        src_const_starts[i] = src->const_start[i];
        src_has_final_value_at[i] =
            (Work_buffer_is_final(src, i) && (src_const_starts[i] < buf_stop));
    }

    float* dest_contents = Work_buffer_get_contents_mut(dest, 0);
    const float* src_contents = Work_buffer_get_contents(src, 0);

    bool dest_has_neg_inf_final_value_at[WORK_BUFFER_SUB_COUNT_MAX] = { false };
    bool src_has_neg_inf_final_value_at[WORK_BUFFER_SUB_COUNT_MAX] = { false };

    for (int i = 0; i < dest->sub_count; ++i)
    {
        dest_has_neg_inf_final_value_at[i] =
            (dest_has_final_value_at[i] &&
             (dest_contents[dest_const_starts[i] * dest->sub_count + i] == -INFINITY));
        src_has_neg_inf_final_value_at[i] =
            (src_has_final_value_at[i] &&
             (src_contents[src_const_starts[i] * src->sub_count + i] == -INFINITY));
    }

    const int32_t actual_start = buf_start * dest->sub_count;
    const int32_t actual_stop = buf_stop * dest->sub_count;
    for (int32_t i = actual_start; i < actual_stop; ++i)
    {
        dassert(!isnan(dest_contents[i]));
        dassert(!isnan(src_contents[i]));
        dest_contents[i] += src_contents[i];
    }

    for (int i = 0; i < dest->sub_count; ++i)
    {
        bool result_is_const_final =
            (dest_has_final_value_at[i] && src_has_final_value_at[i]);
        int32_t new_const_start = max(dest_const_starts[i], src_const_starts[i]);

        // Fill result buffer trail with negative infinity
        // if one of the inputs ends with final negative infinity
        if (dest_has_neg_inf_final_value_at[i])
        {
            result_is_const_final = true;
            new_const_start = min(new_const_start, dest_const_starts[i]);

            for (int32_t k = dest_const_starts[i]; k < buf_stop; ++k)
                dest_contents[k * dest->sub_count] = -INFINITY;
        }

        if (src_has_neg_inf_final_value_at[i])
        {
            result_is_const_final = true;
            new_const_start = min(new_const_start, src_const_starts[i]);

            for (int32_t k = src_const_starts[i]; k < buf_stop; ++k)
                dest_contents[k * dest->sub_count] = -INFINITY;
        }

        dest->is_valid[i] = true;
        Work_buffer_set_const_start(dest, i, new_const_start);
        Work_buffer_set_final(dest, i, result_is_const_final);
    }

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


