

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VECTOR_H
#define K_VECTOR_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/**
 * A dynamically expanding container for objects that do not require internal
 * memory management.
 */
typedef struct Vector Vector;


/**
 * Creates a new Vector.
 *
 * \param elem_size   Size of a single element in bytes -- must be > \c 0.
 *
 * \return   The new Vector if successful, or \c NULL if memory allocation
 *           failed.
 */
Vector* new_Vector(size_t elem_size);


/**
 * Returns the number of elements stored in the Vector.
 *
 * \param v   The Vector -- must not be \c NULL.
 *
 * \return   The number of elements.
 */
size_t Vector_size(const Vector* v);


/**
 * Retrieves an element from the Vector.
 *
 * \param v       The Vector -- must not be \c NULL.
 * \param index   The index of the element -- must be >= \c 0 and
 *                < Vector_size(\a v).
 * \param dest    Destination address for the element -- must not be \c NULL.
 */
void Vector_get(const Vector* v, size_t index, void* dest);


/**
 * Retrieves a reference to an element in the Vector.
 *
 * \param v       The Vector -- must not be \c NULL.
 * \param index   The index of the element -- must be >= \c 0 and
 *                < Vector_size(\a v).
 */
void* Vector_get_ref(const Vector* v, size_t index);


/**
 * Appends an element to the end of the Vector.
 *
 * \param v      The Vector -- must not be \c NULL.
 * \param elem   The element -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Vector_append(Vector* v, void* elem);


/**
 * Destroys an existing Vector.
 *
 * \param v   The Vector, or \c NULL.
 */
void del_Vector(Vector* v);


#endif // K_VECTOR_H


