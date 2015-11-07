

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
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
 * Convert the given pitch from cents to Hz.
 *
 * \param cents   The cents value -- must be finite.
 *
 * \return   The pitch in Hz.
 */
double cents_to_Hz(double cents);


#endif // K_CONVERSIONS_H


