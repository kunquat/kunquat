

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <memory.h>

#include <debug/assert.h>

#include <stdbool.h>
#include <stdint.h>


static int32_t out_of_memory_error_steps = -1;

#define update_out_of_memory_error()            \
    if (true)                                   \
    {                                           \
        if (out_of_memory_error_steps == 0)     \
        {                                       \
            --out_of_memory_error_steps;        \
            return NULL;                        \
        }                                       \
        else if (out_of_memory_error_steps > 0) \
        {                                       \
            --out_of_memory_error_steps;        \
        }                                       \
    } else ignore(0)


static int32_t total_alloc_count = 0;


#define ALIGNED_HEADER_SIZE 1


void* memory_alloc(int64_t size)
{
    rassert(size >= 0);

    if (size == 0)
        return NULL;

    update_out_of_memory_error();

    void* block = malloc((size_t)size);
    if (block != NULL)
        ++total_alloc_count;

    return block;
}


void* memory_calloc(int64_t item_count, int64_t item_size)
{
    rassert(item_count >= 0);
    rassert(item_size >= 0);

    if (item_count == 0 || item_size == 0)
        return NULL;

    update_out_of_memory_error();

    void* block = calloc((size_t)item_count, (size_t)item_size);
    if (block != NULL)
        ++total_alloc_count;

    return block;
}


void* memory_realloc(void* ptr, int64_t size)
{
    rassert(size >= 0);

    if (ptr == NULL)
        return memory_alloc(size);
    else if (size == 0)
        return NULL;

    update_out_of_memory_error();

    void* block = realloc(ptr, (size_t)size);
    if (block != NULL)
        ++total_alloc_count;

    return block;
}


void* memory_alloc_aligned(int64_t size, uint8_t alignment)
{
    rassert(size >= 0);
    rassert(alignment >= 2);
    rassert(alignment <= 64);

    if (size == 0)
        return NULL;

    update_out_of_memory_error();

    const int64_t min_size = size + alignment + ALIGNED_HEADER_SIZE;

    char* block = malloc((size_t)min_size);
    if (block == NULL)
        return NULL;

    ++total_alloc_count;

    const intptr_t header_addr = (intptr_t)block;

    const intptr_t min_user_addr = header_addr + ALIGNED_HEADER_SIZE + 1;
    const intptr_t min_user_rem = min_user_addr % alignment;
    const intptr_t user_addr =
        (min_user_rem == 0) ? min_user_addr : min_user_addr + alignment - min_user_rem;
    rassert(user_addr >= header_addr + ALIGNED_HEADER_SIZE + 1);
    rassert(user_addr <= header_addr + alignment + ALIGNED_HEADER_SIZE + 1);
    rassert(user_addr % alignment == 0);

    const uint8_t offset = (uint8_t)(user_addr - header_addr);
    rassert(offset >= ALIGNED_HEADER_SIZE + 1);
    rassert(min_size - offset >= size);

    block[0] = *(char*)&alignment;

    char* user_block = (char*)user_addr;
    user_block[-1] = *(const char*)&offset;

    return user_block;
}


void memory_free(void* ptr)
{
    free(ptr);
    return;
}


void memory_free_aligned(void* ptr)
{
    if (ptr == NULL)
        return;

    char* block = ptr;
    const uint8_t offset = *(uint8_t*)(&block[-1]);
    rassert(offset >= ALIGNED_HEADER_SIZE + 1);

    char* header = block - offset;
    memory_free(header);

    return;
}


void memory_fake_out_of_memory(int32_t steps)
{
    out_of_memory_error_steps = steps;
    return;
}


int32_t memory_get_alloc_count(void)
{
    return total_alloc_count;
}


