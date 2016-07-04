

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


#ifndef KQT_IRFFT_H
#define KQT_IRFFT_H


#include <stdint.h>
#include <stdlib.h>


/**
 * Fill an array of complex roots of unity for irfft calculation.
 *
 * \param Ws        The array to be filled -- must not be \c NULL and must have
 *                  space for \a tlength / 2 floating-point numbers.
 * \param tlength   The length of the data for which the roots are used for
 *                  -- must be >= \c 4 and a power of 2.
 */
void fill_Ws(float* Ws, int32_t tlength);


/**
 * Calculate the inverse Fast Fourier Transform of a radix-2 half-complex array.
 *
 * \param data     The input/output array -- must not be \c NULL. Upon return,
 *                 the array will contain the real values of the transformed signal.
 *                 The result will not be normalised.
 * \param Ws       The array of complex roots of unity filled by fill_Ws
 *                 -- must not be \c NULL.
 * \param length   The number of elements in \a input -- must be a positive power of 2.
 */
void irfft(float* data, const float* Ws, int32_t length);


#endif // KQT_IRFFT_H


