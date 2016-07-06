

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


#include <mathnum/conversions.h>

#include <debug/assert.h>

#include <math.h>


double dB_to_scale(double dB)
{
    rassert(isfinite(dB) || (dB == -INFINITY));
    return exp2(dB / 6.0);
}


double scale_to_dB(double scale)
{
    rassert(scale >= 0);
    return log2(scale) * 6;
}


double cents_to_Hz(double cents)
{
    rassert(isfinite(cents));
    return exp2(cents / 1200.0) * 440;
}


