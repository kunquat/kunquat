

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


#ifndef K_REAL_H
#define K_REAL_H


#include <stdbool.h>
#include <stdint.h>


/**
 * Real is a combination of rational number and floating-point number types.
 */
typedef struct Real
{
    bool is_frac;
    union
    {
        struct
        {
            int64_t numerator;
            int64_t denominator;
        } frac;
        double doub;
    } fod;
} Real;


/**
 * A new instance of an uninitialised Real object with automatic storage
 * allocation.
 * Useful for passing as a parameter to an initialiser.
 */
#define REAL_AUTO (&(Real){ .fod.doub = 0 })


/**
 * Initialises a Real object as a fraction with the value 1/1.
 *
 * \param real   The Real object -- must not be \c NULL.
 *
 * \return   The parameter \a real.
 */
Real* Real_init(Real* real);


/**
 * Initialises a Real object as a fraction with specified values.
 *
 * \param real          The Real object -- must not be \c NULL.
 * \param numerator     The numerator.
 * \param denominator   The denominator -- must be > \c 0.
 *
 * \return   The parameter \a real.
 */
Real* Real_init_as_frac(
        Real* real,
        int64_t numerator,
        int64_t denominator);


/**
 * Initialises a Real object as a double.
 *
 * \param real     The Real object -- must not be \c NULL.
 * \param double   The double value.
 *
 * \return   The parameter \a real.
 */
Real* Real_init_as_double(Real* real, double val);


/**
 * Tells whether the Real object is stored as a fraction or not.
 *
 * \param real   The Real object -- must be a valid Real.
 *
 * \return   \c true if and only if the Real is stored as a fraction.
 */
bool Real_is_frac(Real* real);


/**
 * Returns the numerator of a Real object.
 * If the Real isn't stored as a fraction, the double value cast to
 * int64_t will be returned.
 *
 * \param real   The Real object -- must be a valid Real.
 *
 * \return   The numerator.
 */
int64_t Real_get_numerator(Real* real);


/**
 * Returns the denominator of a Real object.
 * If the Real isn't stored as a fraction, 1 will be returned.
 *
 * \param real   The Real object -- must be a valid Real.
 *
 * \return   The denominator.
 */
int64_t Real_get_denominator(Real* real);


/**
 * Returns a double approximation of a Real object.
 *
 * \param frac   The Real object -- must be a valid Real.
 *
 * \return   The double value.
 */
double Real_get_double(Real* real);


/**
 * Copies a Real object.
 *
 * \param dest   The destination Real object -- must not be \c NULL.
 * \param src    The source Real object -- must be a valid Real.
 *
 * \return   The parameter \a dest.
 */
Real* Real_copy(Real* dest, Real* src);


/**
 * Multiplies two Real objects.
 * The result will be a double in the following cases:
 * \li One of the operands is a double.
 * \li The product of the numerators is larger than INT64_MAX in magnitude.
 * \li The product of the denominators is larger than INT64_MAX in magnitude.
 *
 * \param ret     The result Real object -- must not be \c NULL.
 * \param real1   The first factor -- must be a valid Real.
 * \param real2   The second factor -- must be a valid Real.
 *
 * \return   The parameter \a ret.
 */
Real* Real_mul(Real* ret, Real* real1, Real* real2);


/**
 * Divides a Real object with another Real object.
 * The result will be a double in the following cases:
 * \li One of the operands is a double.
 * \li The product of the numerator of the dividend and the denominator of the
 *     divisor is larger than INT64_MAX in magnitude.
 * \li The product of the denominator of the dividend and the numerator of the
 *     divisor is larger than INT64_MAX in magnitude.
 *
 * \param ret        The result Real object -- must not be \c NULL.
 * \param dividend   The dividend Real object -- must be a valid Real.
 * \param divisor    The divisor Real object -- must be a valid Real other
 *                   than zero.
 *
 * \return   The parameter \a ret.
 */
Real* Real_div(Real* ret, Real* dividend, Real* divisor);


/**
 * Multiplies a Real object with a floating point value.
 *
 * \param real   The Real factor -- must be a valid Real.
 * \param d      The floating point value.
 *
 * \return   The product as a double value.
 */
double Real_mul_float(Real* real, double d);


/**
 * Compares two Real objects.
 * The comparison is subject to inaccuracies of floating point calculation in
 * the cases listed in the description of Real_div(). However, comparison for
 * equal fractions always returns 0.
 *
 * \param real1   The first Real object -- must be a valid Real.
 * \param real2   The second Real object -- must be a valid Real.
 *
 * \return   Less than, equal to, or greater than zero if \a real1 is found,
 *           respectively, to be less than, equal to, or greater than
 *           \a real2.
 */
int Real_cmp(Real* real1, Real* real2);


#endif // K_REAL_H


