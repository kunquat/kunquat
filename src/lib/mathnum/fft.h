

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


typedef struct FFT_worker
{
    int32_t max_length;
    int32_t cur_length;
    float* wsave;
    int32_t ifac[32];
} FFT_worker;


#define FFT_WORKER_AUTO (&(FFT_worker){ .max_length = 0, .cur_length = 0 })


/**
 * Initialise the FFT worker.
 *
 * \param worker        The FFT worker -- must not be \c NULL.
 * \param max_tlength   Maximum length of Fourier Transform performed
 *                      -- must be positive.
 *
 * \return   \a worker if successful, or \c NULL if memory allocation failed.
 */
FFT_worker* FFT_worker_init(FFT_worker* worker, int32_t max_tlength);


/**
 * Calculate the Fast Fourier Transform of a periodic signal.
 *
 * \param worker   The FFT worker -- must not be \c NULL.
 * \param data     The input/output array -- must not be \c NULL.
 * \param length   The length of the array -- must be positive and not larger
 *                 than the maximum length supported by \a worker.
 */
void FFT_worker_rfft(FFT_worker* worker, float* data, int32_t length);


/**
 * Calculate the inverse Fast Fourier Transform of a periodic signal.
 *
 * \param worker   The FFT worker -- must not be \c NULL.
 * \param data     The input/output array -- must not be \c NULL.
 * \param length   The length of the array -- must be positive and not larger
 *                 than the maximum length supported by \a worker.
 */
void FFT_worker_irfft(FFT_worker* worker, float* data, int32_t length);


/**
 * Deinitialise the FFT worker.
 *
 * \param worker   The FFT worker -- must not be \c NULL.
 */
void FFT_worker_deinit(FFT_worker* worker);


#endif // KQT_FFT_H


