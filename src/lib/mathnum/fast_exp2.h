

/*
 * Authors: Ossi Saresoja, Finland 2016
 *          Tomi Jylhä-Ollila, Finland 2016
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
 * \return   Roughly 2 ^ \a x.
 */
inline double fast_exp2(double x)
{
    assert(isfinite(x));

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
    const int j = i;
    const int k = j & (N - 1);
    return ldexp(b[k] * (x - i + A), j >> K);

#undef K
#undef N
#undef L
#undef A
}


#endif // K_FAST_EXP2_H


