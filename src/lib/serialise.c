

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include <math_common.h>
#include <serialise.h>
#include <xassert.h>


int serialise_bool(char* dest, int size, bool value)
{
    assert(dest != NULL);
    assert(size > 0);
    int printed = snprintf(dest, size, "%s", value ? "true" : "false");
    return MIN(printed, size - 1);
}


int serialise_int(char* dest, int size, int64_t value)
{
    assert(dest != NULL);
    assert(size > 0);
    int printed = snprintf(dest, size, "%" PRId64, value);
    return MIN(printed, size - 1);
}


int serialise_float(char* dest, int size, double value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(isfinite(value));
    double abs_value = fabs(value);
    if (abs_value >= (int64_t)1 << 53 ||
            abs_value < 0.0001)
    {
        int printed = snprintf(dest, size, "%.17e", value);
        return MIN(printed, size - 1);
    }
    char format[] = "%.17f";
    int prec = 18 - log10(abs_value);
    if (prec > 17)
    {
        prec = 17;
    }
    else if (prec < 1)
    {
        prec = 1;
    }
    snprintf(format, strlen(format) + 1, "%%.%df", prec);
    assert(format[strlen(format) - 1] == 'f');
    int printed = snprintf(dest, size, format, value);
    return MIN(printed, size - 1);
}


int serialise_Pat_instance(char* dest, int size, Pat_instance* value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(value != NULL);
    int printed = snprintf(dest, size, "[%" PRId16 ", %" PRId16 "]",
            value->pat, value->inst);
    return MIN(printed, size - 1);
}


int serialise_Real(char* dest, int size, Real* value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(value != NULL);
    if (!Real_is_frac(value))
    {
        int printed = snprintf(dest, size, "[\"f\", ");
        printed = MIN(printed, size - 1);
        printed += serialise_float(dest + printed, size - printed,
                                   Real_get_double(value));
        printed += snprintf(dest + printed, size - printed, "]");
        return MIN(printed, size - 1);
    }
    int printed = snprintf(dest, size, "[\"/\", [%" PRId64 ", %" PRId64 "]]",
                    Real_get_numerator(value), Real_get_denominator(value));
    return MIN(printed, size - 1);
}


int serialise_Timestamp(char* dest, int size, Reltime* value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(value != NULL);
    int printed = snprintf(dest, size, "[%" PRId64 ", %" PRId32 "]",
                       Reltime_get_beats(value), Reltime_get_rem(value));
    return MIN(printed, size - 1);
}


