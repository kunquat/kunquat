

/*
 * Authors: Ossi Saresoja, Finland 2016
 *          Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FAST_EXP2_H
#define KQT_FAST_EXP2_H


#include <debug/assert.h>
#include <intrinsics.h>

#include <limits.h>
#include <math.h>
#include <stdint.h>


/**
 * Calculate a fast approximation of base-2 exponential function.
 *
 * \param x   The input value -- must be finite.
 *
 * \return   Roughly 2 ^ \a x.
 */
static inline double fast_exp2(double x)
{
    dassert(isfinite(x));

#define K 3
#define N (1 << K)
#define L 0.49278062009491144505781798 // L = 1/(1-2^(1/N)) + 1/ln(2^(1/N))
#define A 11.5415603271117072588793974 // A = 1/ln(2^(1/N))

    static const double b[N] =
    {
        0.0866433975699931636771540, // b[i] = 2^(i/N) * ln(2^(1/N))
        0.0944852950344677400302341,
        0.1030369448582453432116499,
        0.1123625851181203074735361,
        0.1225322679335683989642377,
        0.1336223856825675311641740,
        0.1457162448440193058635239,
        0.1589046917773470349548137
    };

    x *= N;
    const double i = floor(x + L);
    dassert(i >= INT_MIN);
    dassert(i <= INT_MAX);
    const int j = (int)i;
    const int k = j & (N - 1);
    return ldexp(b[k] * (x - i + A), j >> K);

#undef K
#undef N
#undef L
#undef A
}


#if KQT_SSE4_1
static inline __m128 fast_exp2_f4(__m128 x)
{
    // Based on:
    // https://stackoverflow.com/questions/47025373/fastest-implementation-of-exponential-function-using-sse
    const __m128 c0 = _mm_set1_ps(0.3371894346f);
    const __m128 c1 = _mm_set1_ps(0.657636276f);
    const __m128 c2 = _mm_set1_ps(1.00172476f);

    const __m128 fl = _mm_floor_ps(x);
    const __m128 rem = _mm_sub_ps(x, fl);
    __m128 p = _mm_mul_ps(c0, rem);
    p = _mm_add_ps(p, c1);
    p = _mm_mul_ps(p, rem);
    p = _mm_add_ps(p, c2);
    const __m128i exp_add = _mm_slli_epi32(_mm_cvtps_epi32(fl), 23);

    const __m128 ret = _mm_castsi128_ps(_mm_add_epi32(exp_add, _mm_castps_si128(p)));

    return ret;
}
#endif


#endif // KQT_FAST_EXP2_H


