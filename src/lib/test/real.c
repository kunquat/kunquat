

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <signal.h>

#include <check.h>

#include <Real.h>


Suite* Real_suite(void);


START_TEST (test_init)
{
    Real real;
    Real* ret = NULL;
    ret = Real_init(&real);
    fail_unless(ret == &real,
            "Real_init() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly by Real_init().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly by Real_init().");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_init_break)
{
    Real_init(NULL);
}
END_TEST
#endif

START_TEST (test_init_as_frac)
{
    Real real;
    Real* ret = NULL;
    real.fod.frac.numerator = -1;
    real.fod.frac.denominator = 0;

    /* 0/x */
    ret = NULL;
    ret = Real_init_as_frac(&real, 0, 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 0,
            "Numerator was not set correctly to 0 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 0, 2);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 0,
            "Numerator was not set correctly to 0 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 0, INT64_MAX - 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 0,
            "Numerator was not set correctly to 0 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 0, INT64_MAX);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 0,
            "Numerator was not set correctly to 0 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    /* 1/x */
    ret = NULL;
    ret = Real_init_as_frac(&real, 1, 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 1, 2);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 2,
            "Denominator was not set correctly to 2 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 1, INT64_MAX - 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == INT64_MAX - 1,
            "Denominator was not set correctly to INT64_MAX - 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 1, INT64_MAX);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == INT64_MAX,
            "Denominator was not set correctly to INT64_MAX by Real_init_as_frac().");

    /* 2/x */
    ret = NULL;
    ret = Real_init_as_frac(&real, 2, 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 2,
            "Numerator was not set correctly to 2 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 2, 2);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 2, INT64_MAX & ~((int64_t)1)); /* gcd = 2 */
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == ((INT64_MAX & ~((int64_t)1)) >> 1),
            "Denominator was not set correctly by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, 2, INT64_MAX | 1); /* gcd = 1 */
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 2,
            "Numerator was not set correctly to 2 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == (INT64_MAX | 1),
            "Denominator was not set correctly by Real_init_as_frac().");

    /* -2/x */
    ret = NULL;
    ret = Real_init_as_frac(&real, -2, 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == -2,
            "Numerator was not set correctly to -2 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, -2, 2);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == -1,
            "Numerator was not set correctly to -1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, -2, INT64_MAX & ~((int64_t)1)); /* gcd = 2 */
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == -1,
            "Numerator was not set correctly to -1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == ((INT64_MAX & ~((int64_t)1)) >> 1),
            "Denominator was not set correctly by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, -2, INT64_MAX | 1); /* gcd = 1 */
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == -2,
            "Numerator was not set correctly to -2 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == (INT64_MAX | 1),
            "Denominator was not set correctly by Real_init_as_frac().");

    /* INT64_MAX/x */
    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MAX, 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == INT64_MAX,
            "Numerator was not set correctly to INT64_MAX by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MAX & ~((int64_t)1), 2); /* gcd = 2 */
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == ((INT64_MAX & ~((int64_t)1)) >> 1),
            "Numerator was not set correctly by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MAX | 1, 2); /* gcd = 1 */
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == (INT64_MAX | 1),
            "Numerator was not set correctly by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 2,
            "Denominator was not set correctly to 2 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MAX, INT64_MAX);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == 1,
            "Numerator was not set correctly to 1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    /* INT64_MIN/x */
    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MIN, 1);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == INT64_MIN,
            "Numerator was not set correctly to INT64_MIN by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MIN, 2);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(((INT64_MIN % 2) != 0) ? (Real_get_numerator(ret) == INT64_MIN)
                : (Real_get_numerator(ret) == (INT64_MIN / 2)),
            "Numerator was not set correctly by Real_init_as_frac().");
    fail_unless(((INT64_MIN % 2) != 0) ? (Real_get_denominator(ret) == 2)
                : (Real_get_denominator(ret) == 1),
            "Denominator was not set correctly by Real_init_as_frac().");

    ret = NULL;
    ret = Real_init_as_frac(&real, INT64_MIN, INT64_MAX);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return the expected object.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless((INT64_MIN == -INT64_MAX) ? (Real_get_numerator(ret) == -1)
                : (Real_get_numerator(ret) == INT64_MIN),
            "Numerator was not set correctly by Real_init_as_frac().");
    fail_unless((INT64_MIN == -INT64_MAX) ? (Real_get_denominator(ret) == 1)
                : (Real_get_denominator(ret) == INT64_MAX),
            "Denominator was not set correctly by Real_init_as_frac().");

    /* -INT64_MAX/INT64_MAX */
    ret = NULL;
    ret = Real_init_as_frac(&real, -INT64_MAX, INT64_MAX);
    fail_unless(ret == &real,
            "Real_init_as_frac() didn't return real.");
    fail_unless(Real_is_frac(ret),
            "Real_init_as_frac() didn't initialise as a fraction.");
    fail_unless(Real_get_numerator(ret) == -1,
            "Numerator was not set correctly to -1 by Real_init_as_frac().");
    fail_unless(Real_get_denominator(ret) == 1,
            "Denominator was not set correctly to 1 by Real_init_as_frac().");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_init_as_frac_break1)
{
    Real_init_as_frac(NULL, 1, 1);
}
END_TEST

START_TEST (test_init_as_frac_break2)
{
    Real real;
    Real_init_as_frac(&real, 1, 0);
}
END_TEST

START_TEST (test_init_as_frac_break3)
{
    Real real;
    Real_init_as_frac(&real, 1, -1);
}
END_TEST

START_TEST (test_init_as_frac_break4)
{
    Real real;
    Real_init_as_frac(&real, 1, INT64_MIN);
}
END_TEST
#endif

START_TEST (test_init_as_double)
{
    Real real;
    Real* ret = NULL;

    ret = Real_init_as_double(&real, -DBL_MAX);
    fail_unless(ret == &real,
            "Real_init_as_double() didn't return the expected object.");
    fail_unless(!Real_is_frac(ret),
            "Real_init_as_double() initialised incorrectly as a fraction.");
    fail_unless(Real_get_double(ret) == -DBL_MAX,
            "Value -DBL_MAX was set incorrectly by Real_init_as_double().");

    ret = NULL;
    ret = Real_init_as_double(&real, -DBL_MIN);
    fail_unless(ret == &real,
            "Real_init_as_double() didn't return the expected object.");
    fail_unless(!Real_is_frac(ret),
            "Real_init_as_double() initialised incorrectly as a fraction.");
    fail_unless(Real_get_double(ret) == -DBL_MIN,
            "Value -DBL_MIN was set incorrectly by Real_init_as_double().");

    ret = NULL;
    ret = Real_init_as_double(&real, 0);
    fail_unless(ret == &real,
            "Real_init_as_double() didn't return the expected object.");
    fail_unless(!Real_is_frac(ret),
            "Real_init_as_double() initialised incorrectly as a fraction.");
    fail_unless(Real_get_double(ret) == 0,
            "Value 0 was set incorrectly by Real_init_as_double().");

    ret = NULL;
    ret = Real_init_as_double(&real, DBL_MIN);
    fail_unless(ret == &real,
            "Real_init_as_double() didn't return the expected object.");
    fail_unless(!Real_is_frac(ret),
            "Real_init_as_double() initialised incorrectly as a fraction.");
    fail_unless(Real_get_double(ret) == DBL_MIN,
            "Value DBL_MIN was set incorrectly by Real_init_as_double().");

    ret = NULL;
    ret = Real_init_as_double(&real, DBL_MAX);
    fail_unless(ret == &real,
            "Real_init_as_double() didn't return the expected object.");
    fail_unless(!Real_is_frac(ret),
            "Real_init_as_double() initialised incorrectly as a fraction.");
    fail_unless(Real_get_double(ret) == DBL_MAX,
            "Value DBL_MAX was set incorrectly by Real_init_as_double().");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_init_as_double_break)
{
    Real_init_as_double(NULL, 0);
}
END_TEST
#endif

START_TEST (test_is_frac)
{
    Real real;
    Real_init_as_frac(&real, 1, 1);
    fail_unless(Real_is_frac(&real),
            "Real_is_frac() incorrectly claimed a frac to be a double.");
    Real_init_as_double(&real, 0);
    fail_unless(!Real_is_frac(&real),
            "Real_is_frac() incorrectly claimed a double to be a frac.");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_is_frac_break)
{
    Real_is_frac(NULL);
}
END_TEST
#endif

START_TEST (test_get_numerator)
{
    Real real;
    Real_init_as_frac(&real, INT64_MIN, 1);
    fail_unless(Real_get_numerator(&real) == INT64_MIN,
            "Real_get_numerator() returned an incorrect numerator.");
    Real_init_as_frac(&real, INT64_MIN + 1, 1);
    fail_unless(Real_get_numerator(&real) == INT64_MIN + 1,
            "Real_get_numerator() returned an incorrect numerator.");
    Real_init_as_frac(&real, -1, 1);
    fail_unless(Real_get_numerator(&real) == -1,
            "Real_get_numerator() returned an incorrect numerator.");
    Real_init_as_frac(&real, 0, 1);
    fail_unless(Real_get_numerator(&real) == 0,
            "Real_get_numerator() returned an incorrect numerator.");
    Real_init_as_frac(&real, 1, 2);
    fail_unless(Real_get_numerator(&real) == 1,
            "Real_get_numerator() returned an incorrect numerator.");
    Real_init_as_frac(&real, INT64_MAX - 1, 1);
    fail_unless(Real_get_numerator(&real) == INT64_MAX - 1,
            "Real_get_numerator() returned an incorrect numerator.");
    Real_init_as_frac(&real, INT64_MAX, 1);
    fail_unless(Real_get_numerator(&real) == INT64_MAX,
            "Real_get_numerator() returned an incorrect numerator.");
    /* The following tests may make incorrect assumptions about C99 */
#if 0
    Real_init_as_double(&real, -DBL_MIN);
    fail_unless(Real_get_numerator(&real) == (long)-DBL_MIN,
            "Real_get_numerator() returned an incorrect numerator of a double.");
    Real_init_as_double(&real, 0.0);
    fail_unless(Real_get_numerator(&real) == (long)0.0,
            "Real_get_numerator() returned an incorrect numerator of a double.");
    Real_init_as_double(&real, DBL_MIN);
    fail_unless(Real_get_numerator(&real) == (long)DBL_MIN,
            "Real_get_numerator() returned an incorrect numerator of a double.");
    /* C doesn't appear to define the following cases precisely */
    Real_init_as_double(&real, -DBL_MAX);
    fail_unless(Real_get_numerator(&real) == (long)-DBL_MAX,
            "Real_get_numerator() returned an incorrect numerator of a double.");
    Real_init_as_double(&real, DBL_MAX);
    fail_unless(Real_get_numerator(&real) == (long)DBL_MAX,
            "Real_get_numerator() returned an incorrect numerator of a double.");
#endif
}
END_TEST

#ifndef NDEBUG
START_TEST (test_get_numerator_break)
{
    Real_get_numerator(NULL);
}
END_TEST
#endif

START_TEST (test_get_denominator)
{
    Real real;
    Real_init_as_frac(&real, 2, 1);
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_get_denominator() returned an incorrect denominator.");
    Real_init_as_frac(&real, 1, 2);
    fail_unless(Real_get_denominator(&real) == 2,
            "Real_get_denominator() returned an incorrect denominator.");
    Real_init_as_frac(&real, 1, INT64_MAX - 1);
    fail_unless(Real_get_denominator(&real) == (INT64_MAX - 1),
            "Real_get_denominator() returned an incorrect denominator.");
    Real_init_as_frac(&real, 1, INT64_MAX);
    fail_unless(Real_get_denominator(&real) == INT64_MAX,
            "Real_get_denominator() returned an incorrect denominator.");
    Real_init_as_double(&real, -DBL_MAX);
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_get_denominator() returned an incorrect denominator of a double.");
    Real_init_as_double(&real, -DBL_MIN);
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_get_denominator() returned an incorrect denominator of a double.");
    Real_init_as_double(&real, 0.0);
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_get_denominator() returned an incorrect denominator of a double.");
    Real_init_as_double(&real, DBL_MIN);
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_get_denominator() returned an incorrect denominator of a double.");
    Real_init_as_double(&real, DBL_MAX);
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_get_denominator() returned an incorrect denominator of a double.");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_get_denominator_break)
{
    Real_get_denominator(NULL);
}
END_TEST
#endif

START_TEST (test_get_double)
{
    Real real;
    Real_init_as_double(&real, -DBL_MAX);
    fail_unless(Real_get_double(&real) == -DBL_MAX,
            "Real_get_double() returned an incorrect value (expected -DBL_MAX).");
    Real_init_as_double(&real, -DBL_MIN);
    fail_unless(Real_get_double(&real) == -DBL_MIN,
            "Real_get_double() returned an incorrect value (expected -DBL_MIN).");
    Real_init_as_double(&real, 0);
    fail_unless(Real_get_double(&real) == 0,
            "Real_get_double() returned an incorrect value (expected 0).");
    Real_init_as_double(&real, DBL_MIN);
    fail_unless(Real_get_double(&real) == DBL_MIN,
            "Real_get_double() returned an incorrect value (expected DBL_MIN).");
    Real_init_as_double(&real, DBL_MAX);
    fail_unless(Real_get_double(&real) == DBL_MAX,
            "Real_get_double() returned an incorrect value (expected DBL_MAX).");
#if 0
    Real_init_as_frac(&real, 0, 1);
    fail_unless(Real_get_double(&real) == ((double)0 / (double)1),
            "Real_get_double() returned an incorrect value of a frac.");
    Real_init_as_frac(&real, 1, 1);
    fail_unless(Real_get_double(&real) == ((double)1 / (double)1),
            "Real_get_double() returned an incorrect value of a frac.");
    Real_init_as_frac(&real, 1, 2);
    fail_unless(Real_get_double(&real) == ((double)1 / (double)2),
            "Real_get_double() returned an incorrect value of a frac.");
#endif
    /* TODO: meaningful tests for fractions */
}
END_TEST

#ifndef NDEBUG
START_TEST (test_get_double_break)
{
    Real_get_double(NULL);
}
END_TEST
#endif

START_TEST (test_copy)
{
    Real dest;
    Real src;
    Real* ret = NULL;
    
    Real_init_as_double(&dest, 0);
    Real_init_as_frac(&src, INT64_MIN, 1);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(Real_is_frac(&dest),
            "Destination is not a fraction despite being copied from one.");
    fail_unless(Real_get_numerator(&dest) == INT64_MIN,
            "Real_copy() failed to copy the numerator.");
    fail_unless(Real_get_denominator(&dest) == 1,
            "Real_copy() failed to copy the denominator.");
    fail_unless(Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_numerator(&src) == INT64_MIN,
            "Real_copy() modified the source.");
    fail_unless(Real_get_denominator(&src) == 1,
            "Real_copy() modified the source.");
    
    ret = NULL;
    Real_init_as_double(&dest, 0);
    Real_init_as_frac(&src, 1, INT64_MAX);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(Real_is_frac(&dest),
            "Destination is not a fraction despite being copied from one.");
    fail_unless(Real_get_numerator(&dest) == 1,
            "Real_copy() failed to copy the numerator.");
    fail_unless(Real_get_denominator(&dest) == INT64_MAX,
            "Real_copy() failed to copy the denominator.");
    fail_unless(Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_numerator(&src) == 1,
            "Real_copy() modified the source.");
    fail_unless(Real_get_denominator(&src) == INT64_MAX,
            "Real_copy() modified the source.");

    ret = NULL;
    Real_init_as_frac(&dest, 1, 2);
    Real_init_as_double(&src, -DBL_MAX);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(!Real_is_frac(&dest),
            "Destination is not a double despite being copied from one.");
    fail_unless(Real_get_double(&dest) == -DBL_MAX,
            "Real_copy() failed to copy the double value.");
    fail_unless(!Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_double(&src) == -DBL_MAX,
            "Real_copy() modified the source.");

    ret = NULL;
    Real_init_as_frac(&dest, 1, 2);
    Real_init_as_double(&src, -DBL_MIN);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(!Real_is_frac(&dest),
            "Destination is not a double despite being copied from one.");
    fail_unless(Real_get_double(&dest) == -DBL_MIN,
            "Real_copy() failed to copy the double value.");
    fail_unless(!Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_double(&src) == -DBL_MIN,
            "Real_copy() modified the source.");

    ret = NULL;
    Real_init_as_frac(&dest, 1, 2);
    Real_init_as_double(&src, 0);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(!Real_is_frac(&dest),
            "Destination is not a double despite being copied from one.");
    fail_unless(Real_get_double(&dest) == 0,
            "Real_copy() failed to copy the double value.");
    fail_unless(!Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_double(&src) == 0,
            "Real_copy() modified the source.");

    ret = NULL;
    Real_init_as_frac(&dest, 1, 2);
    Real_init_as_double(&src, DBL_MIN);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(!Real_is_frac(&dest),
            "Destination is not a double despite being copied from one.");
    fail_unless(Real_get_double(&dest) == DBL_MIN,
            "Real_copy() failed to copy the double value.");
    fail_unless(!Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_double(&src) == DBL_MIN,
            "Real_copy() modified the source.");

    ret = NULL;
    Real_init_as_frac(&dest, 1, 2);
    Real_init_as_double(&src, DBL_MAX);
    ret = Real_copy(&dest, &src);
    fail_unless(ret == &dest,
            "Real_copy() didn't return the destination object.");
    fail_unless(!Real_is_frac(&dest),
            "Destination is not a double despite being copied from one.");
    fail_unless(Real_get_double(&dest) == DBL_MAX,
            "Real_copy() failed to copy the double value.");
    fail_unless(!Real_is_frac(&src),
            "Real_copy() modified the source.");
    fail_unless(Real_get_double(&src) == DBL_MAX,
            "Real_copy() modified the source.");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_copy_break1)
{
    Real real;
    Real_init(&real);
    Real_copy(NULL, &real);
}
END_TEST

START_TEST (test_copy_break2)
{
    Real real;
    Real_init(&real);
    Real_copy(&real, NULL);
}
END_TEST
#endif

START_TEST (test_mul)
{
    Real real1;
    Real real2;
    Real real3;
    Real* ret = NULL;

    ret = NULL;
    Real_init_as_double(&real1, -DBL_MAX);
    Real_init_as_frac(&real2, 0, 1);
    Real_init_as_frac(&real3, 0, 1);
    ret = Real_mul(&real1, &real2, &real3);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(Real_is_frac(&real1),
            "Real_mul() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real1) == 0,
            "Real_mul() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real1) == 1,
            "Real_mul() calculated the denominator incorrectly.");

    ret = NULL;
    Real_init_as_double(&real1, -DBL_MAX);
    Real_init_as_frac(&real2, 5, 4);
    Real_init_as_frac(&real3, 6, 5);
    ret = Real_mul(&real1, &real2, &real3);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(Real_is_frac(&real1),
            "Real_mul() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real1) == 3,
            "Real_mul() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real1) == 2,
            "Real_mul() calculated the denominator incorrectly.");
    fail_unless(Real_is_frac(&real2),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real2) == 5,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real2) == 4,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_is_frac(&real3),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real3) == 6,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real3) == 5,
            "Real_mul() incorrectly changed an operand.");

    ret = NULL;
    Real_init_as_double(&real1, -DBL_MAX);
    Real_init_as_frac(&real2, INT64_MIN / 3, 1);
    Real_init_as_frac(&real3, 2, 1);
    ret = Real_mul(&real1, &real2, &real3);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(Real_is_frac(&real1),
            "Real_mul() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real1) == (INT64_MIN / 3) * 2,
            "Real_mul() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real1) == 1,
            "Real_mul() calculated the denominator incorrectly.");
    fail_unless(Real_is_frac(&real2),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real2) == INT64_MIN / 3,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real2) == 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_is_frac(&real3),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real3) == 2,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real3) == 1,
            "Real_mul() incorrectly changed an operand.");

    // TODO: a better way to test this
    ret = NULL;
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_frac(&real2, 3, 2);
    Real_init_as_frac(&real3, INT64_MAX - 1, INT64_MAX - 2);
    ret = Real_mul(&real1, &real2, &real3);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real1),
            "Real_mul() for large-term fractions didn't produce a double.");
#if 0
    fail_unless(Real_get_double(&real1)
            - (((double)3 / (double)2) * ((double)(INT64_MAX) / (double)991048577))
            < 0.0000001, /* TODO: accuracy level? */
            "Real_mul() didn't calculate large-term fractions accurately enough.");
#endif
    fail_unless(Real_is_frac(&real2),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real2) == 3,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real2) == 2,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_is_frac(&real3),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real3) == INT64_MAX - 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real3) == INT64_MAX - 2,
            "Real_mul() incorrectly changed an operand.");

    ret = NULL;
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_double(&real2, 1);
    Real_init_as_frac(&real3, 1, 1);
    ret = Real_mul(&real1, &real2, &real3);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real1),
            "Real_mul() for double and fraction didn't produce a double.");
    fail_unless(!Real_is_frac(&real2),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_double(&real2) == 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_is_frac(&real3),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real3) == 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real3) == 1,
            "Real_mul() incorrectly changed an operand.");

    ret = NULL;
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_double(&real2, 1);
    Real_init_as_frac(&real3, 1, 1);
    ret = Real_mul(&real1, &real3, &real2); /* order */
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real1),
            "Real_mul() for fraction and double didn't produce a double.");
    fail_unless(!Real_is_frac(&real2),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_double(&real2) == 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_is_frac(&real3),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_numerator(&real3) == 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_denominator(&real3) == 1,
            "Real_mul() incorrectly changed an operand.");

    ret = NULL;
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_double(&real2, 1);
    Real_init_as_double(&real3, 1);
    ret = Real_mul(&real1, &real2, &real3);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real1),
            "Real_mul() for doubles didn't produce a double.");
    fail_unless(!Real_is_frac(&real2),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_double(&real2) == 1,
            "Real_mul() incorrectly changed an operand.");
    fail_unless(!Real_is_frac(&real3),
            "Real_mul() incorrectly changed an operand.");
    fail_unless(Real_get_double(&real3) == 1,
            "Real_mul() incorrectly changed an operand.");

    ret = NULL;
    Real_init_as_frac(&real1, 3, 2);
    ret = Real_mul(&real1, &real1, &real1);
    fail_unless(ret == &real1,
            "Real_mul() didn't return the correct real.");
    fail_unless(Real_is_frac(&real1),
            "Real_mul() didn't produce a frac with one frac as all arguments.");
    fail_unless(Real_get_numerator(&real1) == 9,
            "Real_mul() didn't calculate correctly with one object as all arguments.");
    fail_unless(Real_get_denominator(&real1) == 4,
            "Real_mul() didn't calculate correctly with one object as all arguments.");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_mul_break1)
{
    Real real1;
    Real real2;
    Real_init(&real1);
    Real_init(&real2);
    Real_mul(NULL, &real1, &real2);
}
END_TEST

START_TEST (test_mul_break2)
{
    Real real1;
    Real real2;
    Real_init(&real1);
    Real_init(&real2);
    Real_mul(&real1, NULL, &real2);
}
END_TEST

START_TEST (test_mul_break3)
{
    Real real1;
    Real real2;
    Real_init(&real1);
    Real_init(&real2);
    Real_mul(&real1, &real2, NULL);
}
END_TEST
#endif

START_TEST (test_div)
{
    Real real;
    Real dividend;
    Real divisor;
    Real* ret = NULL;

    ret = NULL;
    Real_init_as_double(&real, -DBL_MAX);
    Real_init_as_frac(&dividend, 0, 1);
    Real_init_as_frac(&divisor, 1, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(Real_is_frac(&real),
            "Real_div() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real) == 0,
            "Real_div() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_div() calculated the denominator incorrectly.");
    fail_unless(Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_numerator(&dividend) == 0,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_denominator(&dividend) == 1,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_numerator(&divisor) == 1,
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_denominator(&divisor) == 1,
            "Real_div() incorrectly changed the divisor.");
    
    ret = NULL;
    Real_init_as_double(&real, -DBL_MAX);
    Real_init_as_frac(&dividend, 1, 1);
    Real_init_as_frac(&divisor, 1, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(Real_is_frac(&real),
            "Real_div() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real) == 1,
            "Real_div() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_div() calculated the denominator incorrectly.");

    ret = NULL;
    Real_init_as_double(&real, -DBL_MAX);
    Real_init_as_frac(&dividend, 1, 1);
    Real_init_as_frac(&divisor, INT64_MAX - 1, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(Real_is_frac(&real),
            "Real_div() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real) == 1,
            "Real_div() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real) == INT64_MAX - 1,
            "Real_div() calculated the denominator incorrectly.");
    fail_unless(Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_numerator(&dividend) == 1,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_denominator(&dividend) == 1,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_numerator(&divisor) == INT64_MAX - 1,
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_denominator(&divisor) == 1,
            "Real_div() incorrectly changed the divisor.");

    ret = NULL;
    Real_init_as_double(&real, -DBL_MIN);
    Real_init_as_frac(&dividend, 5, 4);
    Real_init_as_frac(&divisor, 3, 2);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(Real_is_frac(&real),
            "Real_div() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real) == 5,
            "Real_div() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real) == 6,
            "Real_div() calculated the denominator incorrectly.");
    fail_unless(Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_numerator(&dividend) == 5,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_denominator(&dividend) == 4,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_numerator(&divisor) == 3,
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_denominator(&divisor) == 2,
            "Real_div() incorrectly changed the divisor.");
    
    ret = NULL;
    Real_init_as_double(&real, -DBL_MIN);
    Real_init_as_frac(&dividend, 1, 1);
    Real_init_as_frac(&divisor, -1, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(Real_is_frac(&real),
            "Real_div() for simple fractions didn't produce a fraction.");
    fail_unless(Real_get_numerator(&real) == -1,
            "Real_div() calculated the numerator incorrectly.");
    fail_unless(Real_get_denominator(&real) == 1,
            "Real_div() calculated the denominator incorrectly.");
    fail_unless(Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_numerator(&dividend) == 1,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_denominator(&dividend) == 1,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_numerator(&divisor) == -1,
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_denominator(&divisor) == 1,
            "Real_div() incorrectly changed the divisor.");

    ret = NULL;
    Real_init_as_frac(&real, 2, 3);
    Real_init_as_frac(&dividend, 1, 1);
    Real_init_as_frac(&divisor, INT64_MIN, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real),
            "Real_div() for large-term fractions didn't produce a double.");

    // TODO: a better way to test this
    ret = NULL;
    Real_init_as_frac(&real, 2, 3);
    Real_init_as_frac(&dividend, 9, 8);
    Real_init_as_frac(&divisor, INT64_MAX - 1, INT64_MAX - 2);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real),
            "Real_div() for large-term fractions didn't produce a double.");
#if 0
    fail_unless(Real_get_double(&real)
            - (((double)9 / (double)8) / ((double)500000000 / (double)491048577))
            < 0.00001, /* TODO: accuracy level? */
            "Real_div() didn't calculate large-term fractions accurately enough.");
#endif
    fail_unless(Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_numerator(&dividend) == 9,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_denominator(&dividend) == 8,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_numerator(&divisor) == INT64_MAX - 1,
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_denominator(&divisor) == INT64_MAX - 2,
            "Real_div() incorrectly changed the divisor.");

    ret = NULL;
    Real_init_as_frac(&real, 2, 3);
    Real_init_as_double(&dividend, 1);
    Real_init_as_frac(&divisor, 5, 4);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real),
            "Real_div() for double and fraction didn't produce a double.");
    fail_unless(!Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_double(&dividend) == 1,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_numerator(&divisor) == 5,
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_denominator(&divisor) == 4,
            "Real_div() incorrectly changed the divisor.");
    
    ret = NULL;
    Real_init_as_frac(&real, 2, 3);
    Real_init_as_frac(&dividend, 6, 5);
    Real_init_as_double(&divisor, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real),
            "Real_div() for fraction and double didn't produce a double.");
    fail_unless(Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_numerator(&dividend) == 6,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_denominator(&dividend) == 5,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(!Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_double(&divisor) == 1,
            "Real_div() incorrectly changed the divisor.");
    
    ret = NULL;
    Real_init_as_frac(&real, 2, 3);
    Real_init_as_double(&dividend, 0);
    Real_init_as_double(&divisor, 1);
    ret = Real_div(&real, &dividend, &divisor);
    fail_unless(ret == &real,
            "Real_div() didn't return the correct real.");
    fail_unless(!Real_is_frac(&real),
            "Real_div() for doubles didn't produce a double.");
    fail_unless(!Real_is_frac(&dividend),
            "Real_div() incorrectly changed the dividend.");
    fail_unless(Real_get_double(&dividend) == 0,
            "Real_div() incorrectly changed the dividend.");
    fail_unless(!Real_is_frac(&divisor),
            "Real_div() incorrectly changed the divisor.");
    fail_unless(Real_get_double(&divisor) == 1,
            "Real_div() incorrectly changed the divisor.");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_div_break1)
{
    Real real1;
    Real real2;
    Real_init(&real1);
    Real_init(&real2);
    Real_div(NULL, &real1, &real2);
}
END_TEST

START_TEST (test_div_break2)
{
    Real real1;
    Real real2;
    Real_init(&real1);
    Real_init(&real2);
    Real_div(&real1, NULL, &real2);
}
END_TEST

START_TEST (test_div_break3)
{
    Real real1;
    Real real2;
    Real_init(&real1);
    Real_init(&real2);
    Real_div(&real1, &real2, NULL);
}
END_TEST

START_TEST (test_div_break4)
{
    Real real1;
    Real real2;
    Real real3;
    Real_init_as_frac(&real1, 1, 1);
    Real_init_as_frac(&real2, 1, 1);
    Real_init_as_frac(&real3, 0, 1);
    Real_div(&real1, &real2, &real3);
}
END_TEST
#endif

START_TEST (test_mul_float)
{
    // XXX: This code possibly makes incorrect assumptions about C99 floats.
    Real real;

    Real_init_as_frac(&real, 0, 1);
    fail_unless(Real_mul_float(&real, 0) == 0,
            "Real_mul_float() didn't calculate 0*0 accurately.");

    Real_init_as_frac(&real, 1, 1);
    fail_unless(Real_mul_float(&real, 0) == 0,
            "Real_mul_float() didn't calculate x*0 accurately.");

    Real_init_as_frac(&real, 2, 1);
    fail_unless(Real_mul_float(&real, 2) - 4 < 0.000001,
            "Real_mul_float() didn't calculate accurately enough.");

    Real_init_as_frac(&real, INT64_MIN, 1);
    fail_unless(Real_mul_float(&real, 2) < INT64_MIN,
            "Real_mul_float() messed up with INT64_MIN.");

    Real_init_as_double(&real, 0);
    fail_unless(Real_mul_float(&real, 1) == 0,
            "Real_mul_float() didn't calculate 0*x accurately.");

    Real_init_as_double(&real, 1.0);
    fail_unless(Real_mul_float(&real, 1.0) - (1.0 * 1.0) < 0.000001,
            "Real_mul_float() didn't calculate accurately enough.");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_mul_float_break)
{
    Real_mul_float(NULL, 1);
}
END_TEST
#endif

START_TEST (test_cmp)
{
    Real real1;
    Real real2;

    /* x = x */
    Real_init_as_frac(&real1, INT64_MIN + 1, 1);
    Real_init_as_frac(&real2, INT64_MIN + 1, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed INT64_MIN + 1 = INT64_MIN + 1.");
    Real_init_as_frac(&real1, INT64_MIN + 1, INT64_MAX - 1);
    Real_init_as_frac(&real2, INT64_MIN + 1, INT64_MAX - 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed (INT64_MIN+1)/(INT64_MAX-1) = (INT64_MIN+1)/(INT64_MAX-1).");
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_frac(&real2, -1, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed -1 = -1.");
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_frac(&real2, 0, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed 0 = 0.");
    Real_init_as_frac(&real1, 1, INT64_MAX);
    Real_init_as_frac(&real2, 1, INT64_MAX);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed 1/INT64_MAX = 1/INT64_MAX.");
    Real_init_as_frac(&real1, 1, 1);
    Real_init_as_frac(&real2, 1, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed 1 = 1.");
    Real_init_as_frac(&real1, INT64_MAX, INT64_MAX - 1);
    Real_init_as_frac(&real2, INT64_MAX, INT64_MAX - 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed INT64_MAX/(INT64_MAX-1) = INT64_MAX/(INT64_MAX-1).");
    Real_init_as_frac(&real1, INT64_MAX, 1);
    Real_init_as_frac(&real2, INT64_MAX, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed 0 = 0.");

    Real_init_as_double(&real1, 0);
    Real_init_as_frac(&real2, 0, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with 0 = 0.");
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_double(&real2, 0);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with 0 = 0.");
    Real_init_as_double(&real1, 0);
    Real_init_as_double(&real2, 0);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with 0 = 0.");
    Real_init_as_double(&real1, 1);
    Real_init_as_frac(&real2, 1, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with 1 = 1.");
    Real_init_as_frac(&real1, 1, 1);
    Real_init_as_double(&real2, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with 1 = 1.");
    Real_init_as_double(&real1, 1);
    Real_init_as_double(&real2, 1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with 1 = 1.");
    Real_init_as_double(&real1, -DBL_MAX);
    Real_init_as_double(&real2, -DBL_MAX);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with -DBL_MAX = -DBL_MAX.");
    Real_init_as_double(&real1, -1);
    Real_init_as_double(&real2, -1);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with -1 = -1.");
    Real_init_as_double(&real1, -DBL_MIN);
    Real_init_as_double(&real2, -DBL_MIN);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with -DBL_MIN = -DBL_MIN.");
    Real_init_as_double(&real1, DBL_MIN);
    Real_init_as_double(&real2, DBL_MIN);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with DBL_MIN = DBL_MIN.");
    Real_init_as_double(&real1, DBL_MAX);
    Real_init_as_double(&real2, DBL_MAX);
    fail_unless(Real_cmp(&real1, &real2) == 0,
            "Real_cmp() failed with DBL_MAX = DBL_MAX.");

    /* x != y */
    Real_init_as_frac(&real1, -INT32_MAX, 1);
    Real_init_as_frac(&real2, -INT32_MAX + 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -INT32_MAX < -INT32_MAX + 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -INT32_MAX < -INT32_MAX + 1.");
    Real_init_as_frac(&real1, INT32_MIN + 2, 1);
    Real_init_as_frac(&real2, INT32_MIN + 2, INT32_MAX - 3);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MIN + 2 < (INT32_MIN+2)/(INT32_MAX-3).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MIN + 2 < (INT32_MIN+2)/(INT32_MAX-3).");
    Real_init_as_frac(&real1, INT32_MIN + 2, 1);
    Real_init_as_frac(&real2, -1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MIN + 2 < -1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MIN + 2 < -1.");
    Real_init_as_frac(&real1, INT32_MIN + 2, 1);
    Real_init_as_frac(&real2, 0, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MIN + 2 < 0.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MIN + 2 < 0.");
    Real_init_as_frac(&real1, INT32_MIN + 2, 1);
    Real_init_as_frac(&real2, 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MIN + 2 < 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MIN + 2 < 1.");
    Real_init_as_frac(&real1, INT32_MIN + 2, 1);
    Real_init_as_frac(&real2, INT32_MAX - 2, INT32_MAX - 3);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MIN + 2 < (INT32_MAX-2)/(INT32_MAX-3).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MIN + 2 < (INT32_MAX-2)/(INT32_MAX-3).");
    Real_init_as_frac(&real1, INT32_MIN + 2, 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MIN + 2 < INT32_MAX - 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MIN + 2 < INT32_MAX - 1.");
    Real_init_as_frac(&real1, INT32_MIN + 2, INT32_MAX - 3);
    Real_init_as_frac(&real2, -1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < -1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < -1.");
    Real_init_as_frac(&real1, INT32_MIN + 2, INT32_MAX - 3);
    Real_init_as_frac(&real2, 0, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < 0.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < 0.");
    Real_init_as_frac(&real1, INT32_MIN + 2, INT32_MAX - 3);
    Real_init_as_frac(&real2, 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < 1.");
    Real_init_as_frac(&real1, INT32_MIN + 2, INT32_MAX - 3);
    Real_init_as_frac(&real2, INT32_MAX - 1, INT32_MAX - 2);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < (INT32_MAX-1)/(INT32_MAX-2).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < (INT32_MAX-1)/(INT32_MAX-2).");
    Real_init_as_frac(&real1, INT32_MIN + 2, INT32_MAX - 3);
    Real_init_as_frac(&real2, INT32_MAX - 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < INT32_MAX - 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed (INT32_MIN+2)/(INT32_MAX-3) < INT32_MAX - 1.");
#if 0
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_frac(&real2, -INT32_MAX + 2, INT32_MAX - 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < (-INT32_MAX+2)/(INT32_MAX-1).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < (-INT32_MAX+2)/(INT32_MAX-1).");
#endif
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_frac(&real2, 0, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < 0.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < 0.");
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_frac(&real2, 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < 1.");
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, INT32_MAX - 2);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < (INT32_MAX-1)/(INT32_MAX-2).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < (INT32_MAX-1)/(INT32_MAX-2).");
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < INT32_MAX - 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < INT32_MAX - 1.");
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, INT32_MAX - 2);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 0 < (INT32_MAX-1)/(INT32_MAX-2).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 0 < (INT32_MAX-1)/(INT32_MAX-2).");
    Real_init_as_frac(&real1, 0, 1);
    Real_init_as_frac(&real2, INT32_MAX, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 0 < INT32_MAX.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 0 < INT32_MAX.");
#if 0
    Real_init_as_frac(&real1, INT32_MAX - 2, INT32_MAX - 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, INT32_MAX - 2);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed (INT32_MAX-2)/(INT32_MAX-1) < (INT32_MAX-1)/(INT32_MAX-2).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed (INT32_MAX-2)/(INT32_MAX-1) < (INT32_MAX-1)/(INT32_MAX-2).");
    Real_init_as_frac(&real1, 1, 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, INT32_MAX - 2);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 1 < (INT32_MAX-1)/(INT32_MAX-2).");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 1 < (INT32_MAX-1)/(INT32_MAX-2).");
#endif
    Real_init_as_frac(&real1, 1, 1);
    Real_init_as_frac(&real2, INT32_MAX, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 1 < INT32_MAX.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 1 < INT32_MAX.");
    Real_init_as_frac(&real1, INT32_MAX, INT32_MAX - 1);
    Real_init_as_frac(&real2, INT32_MAX, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MAX/(INT32_MAX-1) < INT32_MAX.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MAX/(INT32_MAX-1) < INT32_MAX.");
    Real_init_as_frac(&real1, INT32_MAX - 2, 1);
    Real_init_as_frac(&real2, INT32_MAX - 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT32_MAX - 2 < INT32_MAX - 1.");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT32_MAX - 2 < INT32_MAX - 1.");

    Real_init_as_double(&real1, -DBL_MAX);
    Real_init_as_frac(&real2, INT64_MIN, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -DBL_MAX < INT64_MIN");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -DBL_MAX < INT64_MIN");
    Real_init_as_double(&real1, -DBL_MAX);
    Real_init_as_double(&real2, INT64_MIN);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -DBL_MAX < INT64_MIN");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -DBL_MAX < INT64_MIN");
    Real_init_as_double(&real1, -1);
    Real_init_as_frac(&real2, -1, 2);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < -1/2");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < -1/2");
    Real_init_as_frac(&real1, -1, 1);
    Real_init_as_double(&real2, -0.5);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < -0.5");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < -0.5");
    Real_init_as_double(&real1, -1);
    Real_init_as_double(&real2, -0.5);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1 < -0.5");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1 < -0.5");
    Real_init_as_frac(&real1, -1, INT64_MAX);
    Real_init_as_double(&real2, -DBL_MIN);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -1/INT64_MAX < -DBL_MIN");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -1/INT64_MAX < -DBL_MIN");
    Real_init_as_double(&real1, -DBL_MIN);
    Real_init_as_double(&real2, DBL_MIN);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed -DBL_MIN < DBL_MIN");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed -DBL_MIN < DBL_MIN");
    Real_init_as_double(&real1, DBL_MIN);
    Real_init_as_frac(&real2, 1, INT64_MAX);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed DBL_MIN < 1/INT64_MAX");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed DBL_MIN < 1/INT64_MAX");
    Real_init_as_frac(&real1, 1, 2);
    Real_init_as_double(&real2, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 1/2 < 1");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 1/2 < 1");
    Real_init_as_double(&real1, 0.5);
    Real_init_as_frac(&real2, 1, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 0.5 < 1");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 0.5 < 1");
    Real_init_as_double(&real1, 0.5);
    Real_init_as_double(&real2, 1);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed 0.5 < 1");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed 0.5 < 1");
    Real_init_as_frac(&real1, INT64_MAX, 1);
    Real_init_as_double(&real2, DBL_MAX);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT64_MAX < DBL_MAX");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT64_MAX < DBL_MAX");
    Real_init_as_double(&real1, INT64_MAX);
    Real_init_as_double(&real2, DBL_MAX);
    fail_unless(Real_cmp(&real1, &real2) < 0,
            "Real_cmp() failed INT64_MAX < DBL_MAX");
    fail_unless(Real_cmp(&real2, &real1) > 0,
            "Real_cmp() failed INT64_MAX < DBL_MAX");
}
END_TEST

#ifndef NDEBUG
START_TEST (test_cmp_break1)
{
    Real real;
    Real_init(&real);
    Real_cmp(NULL, &real);
}
END_TEST

START_TEST (test_cmp_break2)
{
    Real real;
    Real_init(&real);
    Real_cmp(&real, NULL);
}
END_TEST
#endif


Suite* Real_suite(void)
{
    Suite* s = suite_create("Real");
    TCase* tc_init = tcase_create("init");
    TCase* tc_init_as_frac = tcase_create("init_as_frac");
    TCase* tc_init_as_double = tcase_create("init_as_double");
    TCase* tc_is_frac = tcase_create("is_frac");
    TCase* tc_get_numerator = tcase_create("get_numerator");
    TCase* tc_get_denominator = tcase_create("get_denominator");
    TCase* tc_get_double = tcase_create("get_double");
    TCase* tc_copy = tcase_create("copy");
    TCase* tc_mul = tcase_create("mul");
    TCase* tc_div = tcase_create("div");
    TCase* tc_mul_float = tcase_create("mul_float");
    TCase* tc_cmp = tcase_create("cmp");
    suite_add_tcase(s, tc_init);
    suite_add_tcase(s, tc_init_as_frac);
    suite_add_tcase(s, tc_init_as_double);
    suite_add_tcase(s, tc_is_frac);
    suite_add_tcase(s, tc_get_numerator);
    suite_add_tcase(s, tc_get_denominator);
    suite_add_tcase(s, tc_get_double);
    suite_add_tcase(s, tc_copy);
    suite_add_tcase(s, tc_mul);
    suite_add_tcase(s, tc_div);
    suite_add_tcase(s, tc_mul_float);
    suite_add_tcase(s, tc_cmp);

    int timeout = 10;
    tcase_set_timeout(tc_init, timeout);
    tcase_set_timeout(tc_init_as_frac, timeout);
    tcase_set_timeout(tc_init_as_double, timeout);
    tcase_set_timeout(tc_is_frac, timeout);
    tcase_set_timeout(tc_get_numerator, timeout);
    tcase_set_timeout(tc_get_denominator, timeout);
    tcase_set_timeout(tc_get_double, timeout);
    tcase_set_timeout(tc_copy, timeout);
    tcase_set_timeout(tc_mul, timeout);
    tcase_set_timeout(tc_div, timeout);
    tcase_set_timeout(tc_mul_float, timeout);
    tcase_set_timeout(tc_cmp, timeout);

    tcase_add_test(tc_init, test_init);
    tcase_add_test(tc_init_as_frac, test_init_as_frac);
    tcase_add_test(tc_init_as_double, test_init_as_double);
    tcase_add_test(tc_is_frac, test_is_frac);
    tcase_add_test(tc_get_numerator, test_get_numerator);
    tcase_add_test(tc_get_denominator, test_get_denominator);
    tcase_add_test(tc_get_double, test_get_double);
    tcase_add_test(tc_copy, test_copy);
    tcase_add_test(tc_mul, test_mul);
    tcase_add_test(tc_div, test_div);
    tcase_add_test(tc_mul_float, test_mul_float);
    tcase_add_test(tc_cmp, test_cmp);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_init, test_init_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_init_as_frac, test_init_as_frac_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_init_as_frac, test_init_as_frac_break2, SIGABRT);
    tcase_add_test_raise_signal(tc_init_as_frac, test_init_as_frac_break3, SIGABRT);
    tcase_add_test_raise_signal(tc_init_as_frac, test_init_as_frac_break4, SIGABRT);
    
    tcase_add_test_raise_signal(tc_init_as_double, test_init_as_double_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_is_frac, test_is_frac_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_get_numerator, test_get_numerator_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_get_denominator, test_get_denominator_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_get_double, test_get_double_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_copy, test_copy_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_copy, test_copy_break2, SIGABRT);
    
    tcase_add_test_raise_signal(tc_mul, test_mul_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_mul, test_mul_break2, SIGABRT);
    tcase_add_test_raise_signal(tc_mul, test_mul_break3, SIGABRT);
    
    tcase_add_test_raise_signal(tc_div, test_div_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_div, test_div_break2, SIGABRT);
    tcase_add_test_raise_signal(tc_div, test_div_break3, SIGABRT);
    tcase_add_test_raise_signal(tc_div, test_div_break4, SIGABRT);
    
    tcase_add_test_raise_signal(tc_mul_float, test_mul_float_break, SIGABRT);
    
    tcase_add_test_raise_signal(tc_cmp, test_cmp_break1, SIGABRT);
    tcase_add_test_raise_signal(tc_cmp, test_cmp_break2, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Real_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    if (fail_count > 0)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}


