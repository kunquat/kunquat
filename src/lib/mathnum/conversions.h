

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_CONVERSIONS_H
#define KQT_CONVERSIONS_H


#include <debug/assert.h>
#include <intrinsics.h>
#include <mathnum/fast_exp2.h>
#include <mathnum/fast_log2.h>

#include <math.h>


/**
 * Convert the given dB value to scale factor.
 *
 * \param dB   The value in dB -- must be finite or \c -INFINITY.
 *
 * \return   The scale factor.
 */
double dB_to_scale(double dB);


/**
 * Convert the given dB value to scale factor using fast approximation.
 *
 * \param dB   The value in dB -- must be finite or \c -INFINITY.
 *
 * \return   The approximate scale factor.
 */
static inline double fast_dB_to_scale(double dB)
{
    dassert(isfinite(dB) || (dB == -INFINITY));

    if (dB == -INFINITY)
        return 0.0;

    return fast_exp2(dB / 6.0);
}


#if KQT_SSE4_1
static inline __m128 fast_dB_to_scale_f4(__m128 dB)
{
    const __m128 dB_scale = _mm_set1_ps((float)(1.0 / 6.0));
    return fast_exp2_f4(_mm_mul_ps(dB, dB_scale));
}
#endif // KQT_SSE4_1


/**
 * Convert the given scale value to dB.
 *
 * \param scale   The scale value -- must be >= \c 0.
 *
 * \return   The dB value.
 */
double scale_to_dB(double scale);


/**
 * Convert the given scale value to dB using fast approximation.
 *
 * \param scale   The scale value -- must be >= \c 0.
 *
 * \return   The approximate dB value.
 */
static inline double fast_scale_to_dB(double scale)
{
    dassert(scale >= 0);
    if (scale == 0)
        return -INFINITY;

    return fast_log2(scale) * 6;
}


#if KQT_SSE4_1
static inline __m128 fast_scale_to_dB_f4(__m128 scale)
{
    const __m128 zero = _mm_set1_ps(0);
    const __m128 res_mask = _mm_cmpgt_ps(scale, zero);
    const __m128 neg_inf = _mm_set1_ps(-INFINITY);
    const __m128 dB_scale = _mm_set1_ps(6);

    const __m128 result = _mm_mul_ps(fast_log2_f4(scale), dB_scale);

    return _mm_blendv_ps(neg_inf, result, res_mask);
}
#endif // KQT_SSE4_1


/**
 * Convert the given pitch from cents to Hz.
 *
 * \param cents   The cents value -- must be finite.
 *
 * \return   The pitch in Hz.
 */
double cents_to_Hz(double cents);


/**
 * Convert the given pitch from cents to Hz using fast approximation.
 *
 * \param cents   The cents value.
 *
 * \return   The approximate pitch in Hz if \a cents is finite, otherwise \c 0.
 */
static inline double fast_cents_to_Hz(double cents)
{
    if (!isfinite(cents))
        return 0;

    return fast_exp2(cents / 1200.0) * 440;
}


#if KQT_SSE4_1
/**
 * Convert the given pitch values from cents to Hz using fast approximation.
 *
 * \param cents   The cents values.
 *
 * \return   The approximate pitches in Hz if \a cents is in range
 *           [\c -1000000, \c 1000000], otherwise \c 0.
 */
static inline __m128 fast_cents_to_Hz_f4(__m128 cents)
{
    const __m128 min_cents = _mm_set_ps1(-1000000);
    const __m128 max_cents = _mm_set_ps1(1000000);
    const __m128 res_mask = _mm_and_ps(
            _mm_cmpge_ps(cents, min_cents), _mm_cmple_ps(cents, max_cents));

    const __m128 inv_cents = _mm_set_ps1(1.0f / 1200.0f);
    const __m128 a4_hz = _mm_set_ps1(440);

    const __m128 result = _mm_mul_ps(fast_exp2_f4(_mm_mul_ps(cents, inv_cents)), a4_hz);
    return _mm_and_ps(result, res_mask);
}
#endif // KQT_SSE4_1


#endif // KQT_CONVERSIONS_H


