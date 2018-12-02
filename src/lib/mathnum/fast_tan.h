

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FAST_TAN_H
#define KQT_FAST_TAN_H


#include <intrinsics.h>
#include <mathnum/common.h>


#if KQT_SSE

static inline __m128 fast_tan_pos_f4(__m128 x)
{
    const __m128 a = _mm_set1_ps((float)(-4.0 / PI));
    const __m128 b = _mm_set1_ps((float)(4.0 / (PI * PI)));
    const __m128 pi = _mm_set1_ps((float)PI);
    const __m128 hpi = _mm_set1_ps((float)(PI * 0.5));
    const __m128 zero = _mm_set1_ps(0);

    const __m128 x_m_pi = _mm_sub_ps(x, pi);
    const __m128 x_m_hpi = _mm_sub_ps(x, hpi);

    const __m128 approx1_num = _mm_add_ps(
            _mm_mul_ps(a, x_m_pi),
            _mm_mul_ps(_mm_mul_ps(b, x_m_pi), _mm_sub_ps(zero, x_m_pi)));
    const __m128 approx1_den = _mm_add_ps(
            _mm_mul_ps(a, x_m_hpi),
            _mm_mul_ps(_mm_mul_ps(b, x_m_hpi), _mm_sub_ps(zero, x_m_hpi)));
    const __m128 approx1 = _mm_div_ps(approx1_num, approx1_den);

    const __m128 q = _mm_set1_ps(0.775f);
    const __m128 p = _mm_set1_ps(1 - 0.775f);
    const __m128 approx2 = _mm_add_ps(
            _mm_mul_ps(q, approx1), _mm_mul_ps(_mm_mul_ps(p, approx1), approx1));

    return approx2;
}

#endif


#endif // KQT_FAST_TAN_H


