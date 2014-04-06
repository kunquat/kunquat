

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MATHNUM_COMMON_H
#define K_MATHNUM_COMMON_H


#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <debug/assert.h>


#undef PI
#define PI (3.14159265358979323846)


#undef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))


#undef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))


#define undenormalise(x) (fpclassify((x)) != FP_SUBNORMAL ? (x) : 0.0)


/**
 * Tell whether the given number is a power of 2.
 *
 * \param x   The number -- must be > \c 0.
 *
 * \return   \c true if \a x is a power of 2, otherwise \c false.
 */
bool is_p2(int64_t x);


/**
 * Return the next power of 2 of the given number.
 *
 * \param x   The number -- must be > \c 0.
 *
 * \return   The next power of 2.
 */
int64_t next_p2(int64_t x);


/**
 * Return the smallest power of 2 not less than the given number.
 *
 * \param x   The number -- must be > \c 0.
 *
 * \return   The power of 2.
 */
int64_t ceil_p2(int64_t x);


/**
 * Return the integral power with a non-negative exponent.
 *
 * This function evaluates 0^0 to 1.
 *
 * \param base   The base.
 * \param exp    The exponent -- must not be negative.
 *
 * \return   The value base^exp.
 */
int64_t ipowi(int64_t base, int64_t exp);


/**
 * Return the integral power of a floating-point value with a non-negative exponent.
 *
 * This function evaluates 0^0 to 1.
 *
 * \param base   The base.
 * \param exp    The exponent -- must not be negative.
 *
 * \return   The value base^exp.
 */
double powi(double x, int n);


/**
 * Interpolate linearly between two values.
 *
 * \param v1   The first value.
 * \param v2   The second value.
 * \param t    The lerp parameter -- must be >= \c 0 and <= \c 1.
 */
#define lerp(v1, v2, t) \
    (assert((t) >= 0), assert((t) <= 1), (v1) + ((v2) - (v1)) * (t))


/**
 * Sinc function.
 */
double sinc(double x);


#endif // K_MATH_COMMON_H


