

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_MEMORY_H
#define KQT_MEMORY_H


#include <stdint.h>
#include <stdlib.h>


/**
 * A simple memory management interface that supports simulated failures.
 */


/**
 * Shorthands for common operations.
 */
#define memory_alloc_item(type) memory_alloc((int64_t)sizeof(type))
#define memory_alloc_items(type, n) memory_alloc((int64_t)sizeof(type) * (n))
#define memory_calloc_items(type, n) memory_calloc((n), (int64_t)sizeof(type))
#define memory_realloc_items(type, n, ptr) \
    memory_realloc((ptr), (int64_t)sizeof(type) * (n))


/**
 * Allocate a contiguous block of memory.
 *
 * \param size   The amount of bytes to be allocated -- must be >= 0.
 *
 * \return   The starting address of the allocated memory block, or \c NULL if
 *           memory allocation failed or \a size was \c 0.
 */
void* memory_alloc(int64_t size);


/**
 * Allocate an initialised block of memory.
 *
 * \param item_count   Number of items to be allocated -- must be >= \c 0.
 * \param item_size    Size of a single item in bytes -- must be >= \c 0.
 *
 * \return   The starting address of the allocated memory block, or \c NULL if
 *           memory allocation failed or one of the arguments was \c 0.
 */
void* memory_calloc(int64_t item_count, int64_t item_size);


/**
 * Resize a memory block.
 *
 * \param ptr    The starting address of the memory block, or \c NULL.
 * \param size   The new size of the memory block -- must be >= \c 0.
 *
 * \return   The starting address of the resized memory block, or \c NULL if
 *           memory allocation failed or \a size was \c 0.
 */
void* memory_realloc(void* ptr, int64_t size);


/**
 * Free a block of memory.
 *
 * \param ptr   The starting address of the memory block, or \c NULL.
 */
void memory_free(void* ptr);


/**
 * Simulate a memory allocation error on a single allocation request.
 *
 * \param steps   Number of successful allocations that should be made before
 *                the simulated error. Negative value disables error simulation.
 */
void memory_fake_out_of_memory(int32_t steps);


/**
 * Get the total number of successful memory allocations made.
 *
 * \return   The number of allocations made.
 */
int32_t memory_get_alloc_count(void);


#endif // KQT_MEMORY_H


