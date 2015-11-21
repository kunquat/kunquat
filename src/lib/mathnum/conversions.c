

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


#include <math.h>

#include <debug/assert.h>
#include <mathnum/conversions.h>


double dB_to_scale(double dB)
{
    assert(isfinite(dB) || (dB == -INFINITY));
    return exp2(dB / 6.0);
}


double cents_to_Hz(double cents)
{
    assert(isfinite(cents));
    return exp2(cents / 1200.0) * 440;
}

