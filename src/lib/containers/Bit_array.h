

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_BIT_ARRAY_H
#define K_BIT_ARRAY_H


#include <stdbool.h>
#include <stdlib.h>


/**
 * An array of bits.
 */
typedef struct Bit_array Bit_array;


/**
 * Create a new Bit array.
 *
 * \param size   The size of the array -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
Bit_array* new_Bit_array(size_t size);


/**
 * Set a value in the Bit array.
 *
 * \param ba      The Bit array -- must not be \c NULL.
 * \param index   The index of the value -- must be less than the array size.
 * \param value   The new value.
 */
void Bit_array_set(Bit_array* ba, size_t index, bool value);


/**
 * Get a value from the Bit array.
 *
 * \param ba      The Bit array -- must not be \c NULL.
 * \param index   The index of the value -- must be less than the array size.
 *
 * \return   The bit.
 */
bool Bit_array_get(const Bit_array* ba, size_t index);


/**
 * Destroy an existing Bit array.
 *
 * \param ba   The Bit array, or \c NULL.
 */
void del_Bit_array(Bit_array* ba);


#endif // K_BIT_ARRAY_H


