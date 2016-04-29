

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_IRFFT_H
#define K_IRFFT_H


#include <stdlib.h>


/**
 * Calculate the inverse Fast Fourier Transform of a radix-2 half-complex array.
 *
 * \param data     The input/output array -- must not be \c NULL. Upon return,
 *                 the array will contain the real values of the transformed signal.
 *                 The result will not be normalised.
 * \param length   The number of elements in \a input -- must be a positive power of 2.
 */
void irfft(float* array, size_t length);


#endif // K_IRFFT_H


