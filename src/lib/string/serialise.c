

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string/serialise.h>

#include <debug/assert.h>
#include <mathnum/common.h>

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define INT_BUF_SIZE 32


int serialise_bool(char* dest, int size, bool value)
{
    assert(dest != NULL);
    assert(size > 0);

    int printed = snprintf(dest, (size_t)size, "%s", value ? "true" : "false");

    return min(printed, size - 1);
}


int serialise_int(char* dest, int size, int64_t value)
{
    assert(dest != NULL);
    assert(size > 0);

    char result[INT_BUF_SIZE] = "";
    int length = 0;

    if (value == 0)
    {
        result[0] = '0';
    }
    else
    {
        const bool is_negative = (value < 0);

        // Make sure we can get an absolute value
        const bool is_smallest = (value == INT64_MIN);
        const int64_t safe_value = is_smallest ? value + 1 : value;

        const int64_t abs_value = imaxabs(safe_value);

        // Write digits and sign in reverse order
        int64_t left = abs_value;
        while (left != 0)
        {
            assert(length < INT_BUF_SIZE);

            const int64_t digit = left % 10;
            result[length] = (char)(digit + '0');

            left /= 10;
            ++length;
        }

        if (is_negative)
        {
            assert(length < INT_BUF_SIZE);
            result[length] = '-';
            ++length;
        }

        // Fix smallest value
        if (is_smallest)
        {
            assert(result[0] != '9'); // magnitude is a power of 2
            ++result[0];
        }

        // Reverse contents
        for (int i = 0; i < length / 2; ++i)
        {
            int other_index = length - i - 1;
            char tmp = result[i];
            result[i] = result[other_index];
            result[other_index] = tmp;
        }
    }

    int printed = snprintf(dest, (size_t)size, "%s", result);

    return min(printed, size - 1);
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
    const int shift = (int)floor(log10(abs_value));

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
            const int64_t digit = scaled % 10;
            if (digit != 0 || nonzero_found)
            {
                nonzero_found = true;
                digits[i] = (char)(digit + '0');
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
            digits[i] = (char)(digit + '0');

            scaled -= digit;
            assert(scaled >= 0);
        }

        // Remove trailing zeros
        for (int i = (int)strlen(digits) - 1; i >= 1; --i)
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
        serialise_int(&result[cur_len], (int)(sizeof(result) - cur_len - 1), shift);
    }
    else if (shift >= 0)
    {
        const int before_point = shift + 1;
        const int available = (int)strlen(digits);
        strncat(result, digits, (size_t)min(before_point, available));

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

    const int copy_amount = min(size - 1, (int)strlen(result));
    strncpy(dest, result, (size_t)copy_amount);
    dest[copy_amount] = '\0';
    return copy_amount;
}

#undef SIGNIFICANT_MAX


int serialise_Pat_inst_ref(char* dest, int size, const Pat_inst_ref* value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(value != NULL);

    char pat_buf[INT_BUF_SIZE] = "";
    char inst_buf[INT_BUF_SIZE] = "";

    serialise_int(pat_buf, INT_BUF_SIZE, value->pat);
    serialise_int(inst_buf, INT_BUF_SIZE, value->inst);

    int printed = snprintf(dest, (size_t)size, "[%s, %s]", pat_buf, inst_buf);

    return min(printed, size - 1);
}


int serialise_Tstamp(char* dest, int size, const Tstamp* value)
{
    assert(dest != NULL);
    assert(size > 0);
    assert(value != NULL);

    char beats_buf[INT_BUF_SIZE] = "";
    char rem_buf[INT_BUF_SIZE] = "";

    serialise_int(beats_buf, INT_BUF_SIZE, Tstamp_get_beats(value));
    serialise_int(rem_buf, INT_BUF_SIZE, Tstamp_get_rem(value));

    int printed = snprintf(dest, (size_t)size, "[%s, %s]", beats_buf, rem_buf);

    return min(printed, size - 1);
}


