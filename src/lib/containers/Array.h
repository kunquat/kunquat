

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ARRAY_H
#define KQT_ARRAY_H


#include <decl.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * A dynamically expanding container for objects that do not require internal
 * memory management.
 */


/**
 * Create a new Array.
 *
 * \param elem_size   Size of a single element in bytes -- must be > \c 0.
 *
 * \return   The new Array if successful, or \c NULL if memory allocation
 *           failed.
 */
Array* new_Array(int64_t elem_size);


/**
 * Return the number of elements stored in the Array.
 *
 * \param a   The Array -- must not be \c NULL.
 *
 * \return   The number of elements.
 */
int64_t Array_get_size(const Array* a);


/**
 * Get a copy of an element in the Array.
 *
 * \param a       The Array -- must not be \c NULL.
 * \param index   The index of the element -- must be >= \c 0 and
 *                < Array_size(\a a).
 * \param dest    Destination address for the element -- must not be \c NULL.
 */
void Array_get_copy(const Array* a, int64_t index, void* dest);


/**
 * Retrieve a reference to an element in the Array.
 *
 * \param a       The Array -- must not be \c NULL.
 * \param index   The index of the element -- must be >= \c 0 and
 *                < Array_size(\a a).
 *
 * \return   The address of the element.
 */
void* Array_get_ref(const Array* a, int64_t index);


/**
 * Append an element to the end of the Array.
 *
 * \param a      The Array -- must not be \c NULL.
 * \param elem   The element -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Array_append(Array* a, const void* elem);


/**
 * Remove an element at a specified index in the Array.
 *
 * This function shifts the elements following the removed element.
 *
 * \param a       The Array -- must not be \c NULL.
 * \param index   The index of the element to be removed -- must be >= \c 0 and
 *                < Array_size(\a a).
 */
void Array_remove_at(Array* a, int64_t index);


/**
 * Clear the Array.
 *
 * Note that this function might silently fail to shrink its internal memory block, in
 * which case it will continue to use all the memory previously allocated.
 *
 * \param a   The Array -- must not be \c NULL.
 */
void Array_clear(Array* a);


/**
 * Destroy an existing Array.
 *
 * \param a   The Array, or \c NULL.
 */
void del_Array(Array* a);


#endif // KQT_ARRAY_H


