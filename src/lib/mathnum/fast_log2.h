

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FAST_LOG2_H
#define KQT_FAST_LOG2_H


#include <debug/assert.h>
#include <intrinsics.h>

#include <math.h>


/**
 * Calculate a fast approximation of base-2 logarithm function.
 *
 * \param x   The input value -- must be finite and > \c 0.
 *
 * \return   Roughly log2(\a x).
 */
static inline double fast_log2(double x)
{
    dassert(isfinite(x));
    dassert(x > 0);

    // Shift x to range [1, 2)
    int exp = 0;
    const double sx = frexp(x, &exp) * 2.0;
    --exp;

    const double sxmp1 = (sx - 1) / (sx + 1);
    const double sxmp1_2 = sxmp1 * sxmp1;

    static const double fac = 2.8853900817779268; // 2 * (1 / ln(2))
    static const double f13 = (1.0 / 3.0);
    static const double f15 = (1.0 / 5.0);
#define facf13 (fac * f13)
#define facf15 (fac * f15)

    const double l2sx = sxmp1 * (fac + sxmp1_2 * (facf13 + (facf15 * sxmp1_2)));

#undef facf13
#undef facf15

    return l2sx + exp;
}


#if KQT_SSE2
static inline __m128 fast_log2_f4(__m128 x)
{
    // Shift x to range [1, 2)
    const __m128i xp = _mm_castps_si128(x);
    const __m128i exp = _mm_sub_epi32(_mm_srli_epi32(xp, 23), _mm_set1_epi32(127));

    const __m128i mant_mask = _mm_set1_epi32((1 << 23) - 1);
    const __m128i mant_bits = _mm_and_si128(mant_mask, xp);

    const __m128i fr_exp = _mm_set1_epi32(127 << 23);
    const __m128i fr_bits = _mm_or_si128(mant_bits, fr_exp);

    const __m128 sx = _mm_castsi128_ps(fr_bits);

    const __m128 one = _mm_set1_ps(1);
    const __m128 sxmp1 = _mm_div_ps(_mm_sub_ps(sx, one), _mm_add_ps(sx, one));
    const __m128 sxmp1_2 = _mm_mul_ps(sxmp1, sxmp1);

    const __m128 fac = _mm_set1_ps(2.8853900817779268f); // 2 * (1 / ln(2))
    const __m128 facf13 = _mm_set1_ps(2.8853900817779268f / 3.0f);
    const __m128 facf15 = _mm_set1_ps(2.8853900817779268f / 5.0f);

    const __m128 l2sx =
        _mm_mul_ps(sxmp1,
                _mm_add_ps(fac,
                    _mm_mul_ps(sxmp1_2,
                        _mm_add_ps(facf13,
                            _mm_mul_ps(facf15, sxmp1_2)
                            )
                        )
                    )
                );

    return _mm_add_ps(l2sx, _mm_cvtepi32_ps(exp));
}
#endif // KQT_SSE2


#endif // KQT_FAST_LOG2_H


