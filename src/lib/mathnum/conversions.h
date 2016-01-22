

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


#ifndef K_CONVERSIONS_H
#define K_CONVERSIONS_H


#include <debug/assert.h>
#include <mathnum/fast_exp2.h>

#include <math.h>


/**
 * Convert the given dB value to scale factor.
 *
 * \param dB   The value in dB -- must be finite or \c -INFINITY.
 *
 * \return   The scale factor.
 */
double dB_to_scale(double dB);


/**
 * Convert the given dB value to scale factor using fast approximation.
 *
 * \param dB   The value in dB -- must be finite or \c -INFINITY.
 *
 * \return   The scale factor.
 */
inline double fast_dB_to_scale(double dB)
{
    assert(isfinite(dB) || (dB == -INFINITY));

    if (dB == -INFINITY)
        return 0.0;

    return fast_exp2(dB / 6.0);
}


/**
 * Convert the given scale value to dB.
 *
 * \param scale   The scale value -- must be >= \c 0.
 *
 * \return   The dB value.
 */
double scale_to_dB(double scale);


/**
 * Convert the given pitch from cents to Hz.
 *
 * \param cents   The cents value -- must be finite.
 *
 * \return   The pitch in Hz.
 */
double cents_to_Hz(double cents);


/**
 * Convert the given pitch from cents to Hz using fast approximation.
 *
 * \param cents   The cents value.
 *
 * \return   The pitch in Hz if \a cents is finite, otherwise \c 0.
 */
inline double fast_cents_to_Hz(double cents)
{
    if (!isfinite(cents))
        return 0;

    return fast_exp2(cents / 1200.0) * 440;
}


#endif // K_CONVERSIONS_H


