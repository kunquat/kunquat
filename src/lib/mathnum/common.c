

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2011-2016
 *          Ossi Saresoja, Finland 2009-2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <mathnum/common.h>

#include <debug/assert.h>

#include <math.h>


bool is_p2(int64_t x)
{
    assert(x > 0);
    return (x & (x - 1)) == 0;
}


int64_t next_p2(int64_t x)
{
    assert(x > 0);

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;

    return x + 1;
}


int64_t ceil_p2(int64_t x)
{
    assert(x > 0);

    if (is_p2(x))
        return x;

    return next_p2(x);
}


int64_t ipowi(int64_t base, int64_t exp)
{
    assert(exp >= 0);

    if (exp == 0)
        return 1;
    else if (exp % 2 == 0)
        return ipowi(base * base, exp / 2);

    return base * ipowi(base * base, exp / 2);
}


double powi(double base, int exp)
{
    assert(exp >= 0);

    double ret = 1.0;
    while (exp > 0)
    {
        if ((exp & 1) != 0)
            ret *= base;

        exp >>= 1;
        base *= base;
    }

    return ret;
}


double get_range_norm(double value, double start_value, double end_value)
{
    assert(isfinite(value));
    assert(isfinite(start_value));
    assert(isfinite(end_value));

    if (start_value == end_value)
        return 0;

    const double min_value = min(start_value, end_value);
    const double max_value = max(start_value, end_value);
    const double range_width = max_value - min_value;

    const double clamped_value = clamp(value, min_value, max_value);

    double norm = (clamped_value - min_value) / range_width;
    if (start_value > end_value)
        norm = 1.0 - norm;

    return norm;
}


double sinc(double x)
{
    return (x == 0.0) ? 1.0 : (sin(x) / x);
}


