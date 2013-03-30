

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdbool.h>

#include <memory.h>


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
    } else (void)0


void* memory_alloc(size_t size)
{
    if (size == 0)
        return NULL;

    update_out_of_memory_error();

    return malloc(size);
}


void* memory_calloc(size_t item_count, size_t item_size)
{
    if (item_count == 0 || item_size == 0)
        return NULL;

    update_out_of_memory_error();

    return calloc(item_count, item_size);
}


void* memory_realloc(void* ptr, size_t size)
{
    if (ptr == NULL)
        return memory_alloc(size);
    else if (size == 0)
        return NULL;

    update_out_of_memory_error();

    return realloc(ptr, size);
}


void memory_free(void* ptr)
{
    free(ptr);
}


void memory_fake_out_of_memory(int32_t steps)
{
    out_of_memory_error_steps = steps;
}


