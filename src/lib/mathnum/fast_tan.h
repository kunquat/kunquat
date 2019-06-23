

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018-2019
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
    const __m128 sq_pi = _mm_set1_ps((float)(PI * PI));
    const __m128 sq_pi_4 = _mm_div_ps(sq_pi, _mm_set1_ps(4));
    const __m128 xx = _mm_mul_ps(x, x);

    const __m128 a = _mm_sub_ps(_mm_set1_ps(1), _mm_div_ps(_mm_set1_ps(8), sq_pi));
    const __m128 b = _mm_div_ps(_mm_set1_ps(2), _mm_sub_ps(xx, sq_pi_4));

    return _mm_mul_ps(x, _mm_sub_ps(a, b));
}

#endif

static inline double fast_tan(double x)
{
    const double sq_pi = PI * PI;
    return x * ((1 - (8 / sq_pi)) - 2 / (x * x - sq_pi / 4));
}


#endif // KQT_FAST_TAN_H


