

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


#ifndef KQT_FAST_LOG2_H
#define KQT_FAST_LOG2_H


#include <debug/assert.h>

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

    return l2sx + exp;
}


#endif // KQT_FAST_LOG2_H


