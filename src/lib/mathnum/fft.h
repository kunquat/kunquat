

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


#ifndef KQT_FFT_H
#define KQT_FFT_H


#include <stdint.h>
#include <stdlib.h>


/**
 * Initialise work space for real-valued FFT.
 *
 * \param tlength   The length of the transformed array -- must be positive.
 * \param wsave     The array for complex roots of unity -- must not be
 *                  \c NULL and must have space for \a length * 2
 *                  floating-point numbers.
 * \param ifac      The array for factorisation information
 *                  -- must not be \c NULL.
 */
void rfft_init(int32_t tlength, float* wsave, int* ifac);


/**
 * Calculate the Fast Fourier Transform of a periodic signal.
 *
 * \param length   The number of elements to be processed -- must be positive.
 * \param data     The input/output array -- must not be \c NULL.
 * \param wsave    The array of complex roots of unity filled by \a rfft_init
 *                 -- must not be \c NULL.
 * \param ifac     The array of factorisation information filled by
 *                 \a rfft_init -- must not be \c NULL.
 */
void rfft_forward(int32_t length, float* data, float* wsave, const int* ifac);


/**
 * Calculate the inverse Fast Fourier Transform of a periodic signal.
 *
 * \param length   The number of elements to be processed -- must be positive.
 * \param data     The input/output array -- must not be \c NULL.
 * \param wsave    The array of complex roots of unity filled by \a rfft_init
 *                 -- must not be \c NULL.
 * \param ifac     The array of factorisation information filled by
 *                 \a rfft_init -- must not be \c NULL.
 */
void rfft_backward(int32_t length, float* data, float* wsave, const int* ifac);


#endif // KQT_FFT_H


