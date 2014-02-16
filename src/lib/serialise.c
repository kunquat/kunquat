

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
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


#define SIGNIFICANT_MAX 17

int serialise_float(char* dest, int size, double value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(isfinite(value));

    // Note: Not the most accurate conversion, this is a temporary solution...

    if (value == 0.0)
    {
        dest[0] = '0';
        if (size > 1)
            dest[1] = '\0';
        return 1;
    }

    const bool is_negative = (value < 0);
    const double abs_value = fabs(value);
    const int shift = floor(log10(abs_value));

    // Get our most significant digits
    char digits[SIGNIFICANT_MAX + 1] = "";

    if (abs_value >= 1)
    {
        // Normalise so that SIGNIFICANT_MAX digits are above the decimal point
        const double scale_factor = pow(10, SIGNIFICANT_MAX - shift - 1);
        int64_t scaled = (int64_t)round(abs_value * scale_factor);
        assert(scaled >= 0);

        bool nonzero_found = false;
        for (int i = SIGNIFICANT_MAX - 1; i >= 0; --i)
        {
            int64_t digit = scaled % 10;
            if (digit != 0 || nonzero_found)
            {
                nonzero_found = true;
                digits[i] = digit + '0';
            }

            scaled /= 10;
        }
    }
    else
    {
        // Normalise to form 0.d1d2d3..., d1 != 0
        double scaled = abs_value / pow(10, shift + 1);

        for (int i = 0; i < SIGNIFICANT_MAX; ++i)
        {
            if (scaled == 0)
                break;

            scaled *= 10;
            assert(scaled < 10);

            double digit = floor(scaled);
            digits[i] = (int)digit + '0';

            scaled -= digit;
            assert(scaled >= 0);
        }

        // Remove trailing zeros
        for (int i = strlen(digits) - 1; i >= 1; --i)
        {
            if (digits[i] != '0')
                break;

            digits[i] = '\0';
        }
    }

    assert(strlen(digits) > 0);

    // buffer size: '-' + significand + '.' + 'e' + exp + '\0' + safety
    char result[1 + SIGNIFICANT_MAX + 1 + 1 + 3 + 1 + 8] = "";

    if (is_negative)
        result[0] = '-';

    if (shift > 15 || shift < -4)
    {
        // Exponential notation

        // d1.d2d3...
        strncat(result, &digits[0], 1);

        if (strlen(digits) > 1)
        {
            strcat(result, ".");
            strcat(result, &digits[1]);
        }

        // e<exponent>
        strcat(result, "e");
        const size_t cur_len = strlen(result);
        assert(cur_len < sizeof(result));
        serialise_int(&result[cur_len], sizeof(result) - cur_len - 1, shift);
    }
    else if (shift >= 0)
    {
        const int before_point = shift + 1;
        const int available = strlen(digits);
        strncat(result, digits, MIN(before_point, available));

        if (before_point >= available)
        {
            const int add_zeroes = before_point - available;
            for (int i = 0; i < add_zeroes; ++i)
                strcat(result, "0");
        }
        else
        {
            strcat(result, ".");
            strcat(result, &digits[before_point]);
        }
    }
    else
    {
        // Leading zeros
        strcat(result, "0.");
        for (int i = shift + 1; i < 0; ++i)
            strcat(result, "0");

        strcat(result, digits);
    }

    const int copy_amount = MIN(size - 1, (int)strlen(result));
    strncpy(dest, result, copy_amount);
    dest[copy_amount] = '\0';
    return copy_amount;

    /*
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
        prec = 17;
    else if (prec < 1)
        prec = 1;

    snprintf(format, strlen(format) + 1, "%%.%df", prec);
    assert(format[strlen(format) - 1] == 'f');
    int printed = snprintf(dest, size, format, value);

    return MIN(printed, size - 1);
    // */
}

#undef SIGNIFICANT_MAX


int serialise_Pat_inst_ref(char* dest, int size, Pat_inst_ref* value)
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


int serialise_Tstamp(char* dest, int size, Tstamp* value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(value != NULL);

    int printed = snprintf(dest, size, "[%" PRId64 ", %" PRId32 "]",
            Tstamp_get_beats(value), Tstamp_get_rem(value));

    return MIN(printed, size - 1);
}


