

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
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#include <Real.h>


/**
 * Reduces a Real object into lowest terms.
 *
 * Does nothing unless the Real object is a fraction.
 *
 * \param real   The Real object -- must not be \c NULL.
 *
 * \return   The parameter \a real.
 */
static Real* Real_normalise(Real* real);


#ifndef NDEBUG
    /**
     * Validates a Real object.
     *
     * \param real   The Real object.
     *
     * \return   \c true if and only if the Real object is valid.
     */
    static bool Real_validate_(Real* real);
    #define Real_validate(real)   ( assert(Real_validate_(real)) )
#else
    #define Real_validate(real)   ((void)0)
#endif


/**
 * Performs integer multiplication with overflow check.
 *
 * \param a     The first term.
 * \param b     The second term.
 * \param res   Pointer to the result variable -- must not be \c NULL. If an
 *              overflow would occur, the stored value will not be changed.
 *
 * \return   \c true if \a a * \a b was calculated successfully, or \c false
 *           if the result would become larger than INT64_MAX in magnitude.
 */
static bool safe_mul(int64_t a, int64_t b, int64_t* res);


Real* Real_init(Real* real)
{
    assert(real != NULL);
    return Real_init_as_frac(real, 1, 1);
}


Real* Real_init_as_frac(Real* real, int64_t numerator, int64_t denominator)
{
    assert(real != NULL);
    assert(denominator > 0);
    real->is_frac = 1;
    real->fod.frac.numerator = numerator;
    real->fod.frac.denominator = denominator;
    return Real_normalise(real);
}


Real* Real_init_as_double(Real* real, double val)
{
    assert(real != NULL);
    real->is_frac = 0;
    real->fod.doub = val;
    return real;
}


bool Real_is_frac(Real* real)
{
    Real_validate(real);
    return real->is_frac;
}


int64_t Real_get_numerator(Real* real)
{
    Real_validate(real);
    if (!real->is_frac)
    {
        return (int64_t)real->fod.doub;
    }
    return real->fod.frac.numerator;
}


int64_t Real_get_denominator(Real* real)
{
    Real_validate(real);
    if (!real->is_frac)
    {
        return 1;
    }
    return real->fod.frac.denominator;
}


double Real_get_double(Real* real)
{
    Real_validate(real);
    if (real->is_frac)
    {
        return (double)real->fod.frac.numerator
                / (double)real->fod.frac.denominator;
    }
    return real->fod.doub;
}


Real* Real_copy(Real* dest, Real* src)
{
    assert(dest != NULL);
    Real_validate(src);
    if (src->is_frac)
    {
        dest->is_frac = 1;
        dest->fod.frac.numerator = src->fod.frac.numerator;
        dest->fod.frac.denominator = src->fod.frac.denominator;
        Real_validate(dest);
        return dest;
    }
    dest->is_frac = 0;
    dest->fod.doub = src->fod.doub;
    Real_validate(dest);
    return dest;
}


Real* Real_mul(Real* ret, Real* real1, Real* real2)
{
    assert(ret != NULL);
    Real_validate(real1);
    Real_validate(real2);
    if (real1->is_frac && real2->is_frac)
    {
        int64_t num = 0;
        int64_t den = 0;
        if (safe_mul(real1->fod.frac.numerator,
                    real2->fod.frac.numerator, &num)
                && safe_mul(real1->fod.frac.denominator,
                    real2->fod.frac.denominator, &den))
        {
            ret->is_frac = 1;
            ret->fod.frac.numerator = num;
            ret->fod.frac.denominator = den;
            return Real_normalise(ret);
        }
    }
    return Real_init_as_double(ret, Real_get_double(real1) * Real_get_double(real2));
}


Real* Real_div(Real* ret, Real* dividend, Real* divisor)
{
    assert(ret != NULL);
    Real_validate(dividend);
    Real_validate(divisor);
    if (dividend->is_frac && divisor->is_frac)
    {
        assert(divisor->fod.frac.numerator != 0);
        int64_t num = 0;
        int64_t den = 0;
        if (safe_mul(dividend->fod.frac.numerator,
                    divisor->fod.frac.denominator, &num)
                && safe_mul(dividend->fod.frac.denominator,
                    divisor->fod.frac.numerator, &den))
        {
            if (den < 0)
            {
                num = -num;
                den = -den;
            }
            ret->is_frac = 1;
            ret->fod.frac.numerator = num;
            ret->fod.frac.denominator = den;
            return Real_normalise(ret);
        }
    }
    assert(Real_get_double(divisor) != 0);
    return Real_init_as_double(ret, Real_get_double(dividend) / Real_get_double(divisor));
}


double Real_mul_float(Real* real, double d)
{
    Real_validate(real);
    return Real_get_double(real) * d;
}


int Real_cmp(Real* real1, Real* real2)
{
    Real_validate(real1);
    Real_validate(real2);
    if (real1->is_frac && real2->is_frac)
    {
        int64_t num1 = real1->fod.frac.numerator;
        int64_t den1 = real1->fod.frac.denominator;
        int64_t num2 = real2->fod.frac.numerator;
        int64_t den2 = real2->fod.frac.denominator;
        if (num1 == num2 && den1 == den2)
        {
            return 0;
        }
        int64_t term1 = 0;
        int64_t term2 = 0;
        if (safe_mul(num1, den2, &term1)
                && safe_mul(num2, den1, &term2))
        {
            if (term1 < term2)
                return -1;
            else if (term1 > term2)
                return 1;
            return 0;
        }
    }
    double val1 = Real_get_double(real1);
    double val2 = Real_get_double(real2);
    if (val1 < val2)
    {
        return -1;
    }
    else if (val1 > val2)
    {
        return 1;
    }
    return 0;
}


Real* Real_normalise(Real* real)
{
    Real_validate(real);
    if (!real->is_frac)
    {
        return real;
    }
    if (real->fod.frac.numerator == INT64_MIN)
    {
        // get rid of INT64_MIN for imaxabs()
        if ((INT64_MIN % 2) == -1)
        {
            // Potentially suboptimal: real may not be in lowest terms here
            // if INT_LEAST64_MIN is used.
            return real;
        }
        if ((real->fod.frac.denominator & 1) == 0)
        {
            real->fod.frac.numerator = INT64_MIN / 2;
            real->fod.frac.denominator >>= 1;
        }
        else
        {
            // INT64_MIN is has the form -2^n,
            // so the fraction is irreducible here.
            // Potentially suboptimal if INT_LEAST64_MIN is used.
            return real;
        }
    }
    int k = 0;
    int64_t num = imaxabs(real->fod.frac.numerator);
    int64_t den = real->fod.frac.denominator;
    if (num == 0)
    {
        real->fod.frac.denominator = 1;
        return real;
    }
    while ((num & 1) == 0 && (den & 1) == 0)
    {
        num >>= 1;
        den >>= 1;
        ++k;
    }
    do
    {
        if ((den & 1) == 0)
            den >>= 1;
        else if ((num & 1) == 0)
            num >>= 1;
        else if (den >= num)
            den = (den - num) >> 1;
        else
            num = (num - den) >> 1;
    } while (den > 0);
    assert(num > 0);
    num <<= k;
    real->fod.frac.numerator /= num;
    real->fod.frac.denominator /= num;
    Real_validate(real);
    return real;
}


#ifndef NDEBUG
bool Real_validate_(Real* real)
{
    if (real == NULL)
    {
        return false;
    }
    if (real->is_frac)
    {
        if (real->fod.frac.denominator <= 0)
        {
            return false;
        }
    }
    else
    {
        if (isnan(real->fod.doub))
        {
            return false;
        }
    }
    return true;
}
#endif


// Works the way described in http://www.fefe.de/intof.html
static bool safe_mul(int64_t a, int64_t b, int64_t* res)
{
    assert(res != NULL);
    bool negative = false;
    if ((a == INT64_MIN || b == INT64_MIN)
            && a != 0 && b != 0
            && INT64_MIN < -INT64_MAX)
    {
        return false;
    }
    if (a < 0)
    {
        a = -a;
        negative = !negative;
    }
    if (b < 0)
    {
        b = -b;
        negative = !negative;
    }
    uint64_t ahi = (uint64_t)a >> 32;
    uint64_t bhi = (uint64_t)b >> 32;
    if (ahi != 0 && bhi != 0)
    {
        return false;
    }
    uint64_t alo = a & 0xffffffffULL;
    uint64_t blo = b & 0xffffffffULL;
    uint64_t hi = (ahi * blo) + (alo * bhi);
    if (hi > 0x7fffffffULL)
    {
        return false;
    }
    hi <<= 32;
    uint64_t lo = alo * blo;
    if (UINT64_MAX - lo < hi || INT64_MAX < hi + lo)
    {
        return false;
    }
    *res = hi + lo;
    if (negative)
    {
        *res = -*res;
    }
    return true;
}


