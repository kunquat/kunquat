

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MATH_COMMON_H
#define K_MATH_COMMON_H


#include <stdbool.h>
#include <stdint.h>
#include <math.h>


#undef PI
#define PI (3.14159265358979323846)


#undef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))


#undef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))


#define undenormalise(x) (fpclassify((x)) != FP_SUBNORMAL ? (x) : 0.0)


/**
 * Tells whether the given number is a power of 2.
 *
 * \param x   The number -- must be > \c 0.
 *
 * \return   \c true if \a x is a power of 2, otherwise \c false.
 */
bool is_p2(int64_t x);


/**
 * Returns the next power of 2 of the given number.
 *
 * \param x   The number -- must be > \c 0.
 *
 * \return   The next power of 2.
 */
int64_t next_p2(int64_t x);


/**
 * Returns the smallest power of 2 not less than the given number.
 *
 * \param x   The number -- must be > \c 0.
 *
 * \return   The power of 2.
 */
int64_t ceil_p2(int64_t x);


#endif // K_MATH_COMMON_H


