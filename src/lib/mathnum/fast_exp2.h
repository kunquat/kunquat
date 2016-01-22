

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


#ifndef K_FAST_EXP2_H
#define K_FAST_EXP2_H


#include <debug/assert.h>

#include <math.h>
#include <stdint.h>


/**
 * Calculate a fast approximation of base-2 exponential function.
 *
 * \param x   The input value -- must be finite.
 *
 * \return   2 ^ \a x.
 */
inline double fast_exp2(double x)
{
    assert(isfinite(x));

    // TODO: Not that much faster than exp2 really, this needs more work!

#define KQT_FAST_EXP2_LN2   0.693147180559945309
#define KQT_FAST_EXP2_LN2_2 0.480453013918201424
#define KQT_FAST_EXP2_LN3_2 0.333024651988929479
    static const double f2 = 0.5 * KQT_FAST_EXP2_LN2_2;
    //static const double f3 = (1.0 / 6.0) * KQT_FAST_EXP2_LN3_2;

    // For all fast_exp2(x) > 0, 2048 shifts x to positive side to avoid stupid fmod
    static const double pre_shift = 2048.0 + 0.5;

    // Shift x to range [-0.5, 0.5)
    const double a = x + pre_shift;
    const double fa = floor(a);
    const double sa = a - fa;
    const double sx = sa - 0.5;
    const int shift_amount = fa - 2048;

    const double sx2 = sx * sx;
    //const double sx3 = sx2 * sx;

    const double esx = 1 + (sx * KQT_FAST_EXP2_LN2) + (sx2 * f2);
#undef KQT_FAST_EXP2_LN2
#undef KQT_FAST_EXP2_LN2_2
#undef KQT_FAST_EXP2_LN3_2

    return ldexp(esx, shift_amount);
}


#endif // K_FAST_EXP2_H


