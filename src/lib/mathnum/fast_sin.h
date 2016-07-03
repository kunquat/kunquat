

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FAST_SIN_H
#define KQT_FAST_SIN_H


#include <mathnum/common.h>

#include <math.h>


/**
 * Calculate a fast approximation of sine.
 *
 * \param x   The input value -- must be within range [0, 2 * \c PI].
 *
 * \return   The sine of \a x.
 */
static inline double fast_sin(double x)
{
    assert(x >= 0);
    assert(x <= 2.0 * PI);

    // Use a parabolic approximation based on:
    // http://forum.devmaster.net/t/fast-and-accurate-sine-cosine/9648
    // (reworked for our argument range)

    static const double a = -4.0 / PI;
    static const double b = 4.0 / (PI * PI);
    const double x_m_pi = x - PI;

    const double approx1 = (a * x_m_pi) + (b * x_m_pi * fabs(x_m_pi));

#define Q 0.775
#define P (1 - Q)
    const double approx2 = Q * approx1 + P * approx1 * fabs(approx1);
#undef Q
#undef P

    return approx2;
}


#endif // KQT_FAST_SIN_H


