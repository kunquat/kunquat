

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string/Streader.h>

#include <debug/assert.h>
#include <kunquat/limits.h>

#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


Streader* Streader_init(Streader* sr, const char* str, int64_t len)
{
    assert(sr != NULL);
    assert(str != NULL || len == 0);
    assert(len >= 0);

    sr->pos = 0;
    sr->len = len;
    sr->line = 1;
    sr->str = str;
    sr->error = *ERROR_AUTO;

    return sr;
}


bool Streader_is_error_set(const Streader* sr)
{
    assert(sr != NULL);
    return Error_is_set(&sr->error);
}


const char* Streader_get_error_desc(const Streader* sr)
{
    assert(sr != NULL);
    return Error_get_desc(&sr->error);
}


void Streader_set_error(Streader* sr, const char* format, ...)
{
    assert(sr != NULL);
    assert(format != NULL);

    va_list args;
    va_start(args, format);
    Error_set_desc_va_list(
            &sr->error, ERROR_FORMAT, "", sr->line, "", format, args);
    va_end(args);

    return;
}


void Streader_set_memory_error(Streader* sr, const char* format, ...)
{
    assert(sr != NULL);
    assert(format != NULL);

    va_list args;
    va_start(args, format);
    Error_set_desc_va_list(
            &sr->error, ERROR_MEMORY, "", sr->line, "", format, args);
    va_end(args);

    return;
}


void Streader_clear_error(Streader* sr)
{
    assert(sr != NULL);
    Error_clear(&sr->error);
    return;
}


static bool Streader_end_reached(const Streader* sr)
{
    assert(sr != NULL);
    assert(sr->pos <= sr->len);

    return sr->pos == sr->len;
}


#define CUR_CH (assert(!Streader_end_reached(sr)), sr->str[sr->pos])


const char* Streader_get_remaining_data(const Streader* sr)
{
    assert(sr != NULL);

    if (Streader_end_reached(sr))
        return NULL;

    return &sr->str[sr->pos];
}


static void print_wrong_char(char dest[5], char ch)
{
    assert(dest != NULL);

    if (isprint(ch))
        snprintf(dest, 5, "'%c'", ch);
    else
        snprintf(dest, 5, "%#04hhx", (unsigned char)ch);

    return;
}


bool Streader_skip_whitespace(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    while (!Streader_end_reached(sr) && isspace(CUR_CH))
    {
        if (CUR_CH == '\n')
            ++sr->line;

        ++sr->pos;
    }

    assert(sr->pos <= sr->len);

    return true;
}


bool Streader_has_data(Streader* sr)
{
    assert(sr != NULL);
    assert(!Streader_is_error_set(sr));

    Streader_skip_whitespace(sr);
    return !Streader_end_reached(sr);
}


bool Streader_match_char(Streader* sr, char ch)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    if (Streader_end_reached(sr))
    {
        Streader_set_error(sr, "Expected '%c' instead of end of data", ch);
        return false;
    }

    if (CUR_CH != ch)
    {
        if (isprint(CUR_CH))
            Streader_set_error(
                    sr, "Expected '%c' instead of '%c'", ch, CUR_CH);
        else
            Streader_set_error(
                    sr,
                    "Expected '%c' instead of %#04hhx",
                    ch, (unsigned)CUR_CH);

        return false;
    }

    ++sr->pos;

    return true;
}


bool Streader_try_match_char(Streader* sr, char ch)
{
    assert(sr != NULL);
    assert(!Streader_is_error_set(sr));

    const int64_t start_pos = sr->pos;
    const bool success = Streader_match_char(sr, ch);
    if (!success)
    {
        Streader_clear_error(sr);
        sr->pos = start_pos;
    }

    return success;
}


static bool Streader_match_char_seq(Streader* sr, const char* seq)
{
    assert(sr != NULL);
    assert(seq != NULL);

    assert(!Streader_is_error_set(sr));
    assert(!Streader_end_reached(sr));

    // Check that we have enough data
    const int64_t expected_len = (int64_t)strlen(seq);
    if (sr->len - sr->pos < expected_len)
    {
        Streader_set_error(
                sr,
                "Too few bytes left to contain expected string `%s`",
                seq);
        return false;
    }

    // Match the string
    if (strncmp(&sr->str[sr->pos], seq, (size_t)expected_len) != 0)
    {
        Streader_set_error(sr, "Unexpected string (expected `%s`)", seq);
        return false;
    }

    // Update position
    for (int64_t i = 0; i < expected_len; ++i)
    {
        if (CUR_CH == '\n')
            ++sr->line;
        ++sr->pos;
    }

    if (!Streader_end_reached(sr) && isalnum(CUR_CH))
    {
        Streader_set_error(
                sr, "Unexpected character after expected string `%s`", seq);
        return false;
    }

    return true;
}


static bool Streader_try_match_char_seq(Streader* sr, const char* seq)
{
    assert(sr != NULL);
    assert(!Streader_is_error_set(sr));
    assert(seq != NULL);

    const int64_t start_pos = sr->pos;
    const bool success = Streader_match_char_seq(sr, seq);
    if (!success)
    {
        Streader_clear_error(sr);
        sr->pos = start_pos;
    }

    return success;
}


bool Streader_match_string(Streader* sr, const char* str)
{
    assert(sr != NULL);
    assert(str != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    if (Streader_end_reached(sr))
    {
        Streader_set_error(
                sr, "Expected beginning of a string instead of end of data");
        return false;
    }

    // Match opening double quote
    if (CUR_CH != '\"')
    {
        if (isprint(CUR_CH))
            Streader_set_error(
                    sr,
                    "Expected beginning of a string instead of '%c'",
                    CUR_CH);
        else
            Streader_set_error(
                    sr,
                    "Expected beginning of a string instead of %#2x",
                    (unsigned)CUR_CH);

        return false;
    }

    ++sr->pos;

    // Match the string
    if (!Streader_match_char_seq(sr, str))
        return false;

    // Match closing double quote
    if (Streader_end_reached(sr) || CUR_CH != '\"')
    {
        Streader_set_error(sr, "Unexpected string (expected `%s`)", str);
        return false;
    }

    ++sr->pos;

    return true;
}


bool Streader_read_null(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    if (!Streader_match_char_seq(sr, "null"))
    {
        Streader_set_error(sr, "Expected null");
        return false;
    }

    return true;
}


bool Streader_read_bool(Streader* sr, bool* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    bool result = false;

    if (Streader_try_match_char_seq(sr, "true"))
    {
        result = true;
    }
    else
    {
        if (!Streader_match_char_seq(sr, "false"))
        {
            Streader_set_error(sr, "Expected a boolean value");
            return false;
        }
    }

    if (dest != NULL)
        *dest = result;

    return true;
}


#define NONZERO_DIGITS "123456789"

bool Streader_read_int(Streader* sr, int64_t* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    int64_t result = 0;
    const bool negative = Streader_try_match_char(sr, '-');

    if (Streader_end_reached(sr))
    {
        Streader_set_error(sr, "Unexpected end of data");
        return false;
    }

    if (CUR_CH == '0')
    {
        ++sr->pos;
        if (!Streader_end_reached(sr) && isalnum(CUR_CH))
        {
            Streader_set_error(sr, "Unexpected characters after initial zero");
            return false;
        }
    }
    else if (strchr(NONZERO_DIGITS, CUR_CH) != NULL)
    {
        if (negative)
        {
            static const int64_t safe_lower = INT64_MIN / 10;

            while (!Streader_end_reached(sr) && isdigit(CUR_CH))
            {
                // Check for multiplication overflow
                if (result < safe_lower)
                {
                    Streader_set_error(sr, "Integer is too small");
                    return false;
                }
                result *= 10;

                const int64_t contrib = CUR_CH - '0';

                // Check for addition overflow
                if (INT64_MIN + contrib > result)
                {
                    Streader_set_error(sr, "Integer is too small");
                    return false;
                }
                result -= contrib;

                ++sr->pos;
            }
        }
        else
        {
            static const int64_t safe_upper = INT64_MAX / 10;

            while (!Streader_end_reached(sr) && isdigit(CUR_CH))
            {
                // Check for multiplication overflow
                if (result > safe_upper)
                {
                    Streader_set_error(sr, "Integer is too large");
                    return false;
                }
                result *= 10;

                const int64_t contrib = CUR_CH - '0';

                // Check for addition overflow
                if (INT64_MAX - contrib < result)
                {
                    Streader_set_error(sr, "Integer is too large");
                    return false;
                }
                result += contrib;

                ++sr->pos;
            }
        }
    }
    else
    {
        Streader_set_error(sr, "No digits found");
        return false;
    }

    if (dest != NULL)
        *dest = result;

    return true;
}


#define SIGNIFICANT_MAX 17

bool Streader_read_float(Streader* sr, double* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    // Note: Not completely accurate, but not supposed to be portable anyway...

    bool is_negative = false;
    int64_t significand = 0;
    int significand_shift = 0;
    int significant_digits_read = 0;

    int exponent = 0;
    bool exponent_is_negative = false;

    // Negation
    if (Streader_try_match_char(sr, '-'))
        is_negative = true;

    if (Streader_end_reached(sr))
    {
        Streader_set_error(sr, "No digits found");
        return false;
    }

    // Significand
    if (CUR_CH == '0')
    {
        ++sr->pos;
    }
    else if (strchr(NONZERO_DIGITS, CUR_CH) != NULL)
    {
        while (!Streader_end_reached(sr) && isdigit(CUR_CH))
        {
            if (significant_digits_read < SIGNIFICANT_MAX)
            {
                // Store the digit into our significand
                assert(significand < INT64_MAX / 10);
                significand *= 10;
                significand += (int64_t)(CUR_CH - '0');
                ++significant_digits_read;
            }
            else
            {
                // We have run out of accuracy, just update magnitude
                ++significand_shift;
            }

            ++sr->pos;
        }
    }
    else
    {
        Streader_set_error(sr, "No digits found");
        return false;
    }

    // Decimal part
    if (!Streader_end_reached(sr) && CUR_CH == '.')
    {
        ++sr->pos;

        while (!Streader_end_reached(sr) && isdigit(CUR_CH))
        {
            if (significant_digits_read < SIGNIFICANT_MAX)
            {
                // Append digit to our significand and compensate in shift
                assert(significand < INT64_MAX / 10);
                significand *= 10;
                significand += (int64_t)(CUR_CH - '0');
                --significand_shift;

                if (significand != 0)
                    ++significant_digits_read;
            }

            ++sr->pos;
        }

        // Require at least one digit
        if (sr->str[sr->pos - 1] == '.')
        {
            Streader_set_error(sr, "No digits found after decimal point");
            return false;
        }
    }

    // Exponent part
    if (!Streader_end_reached(sr) &&
            ((CUR_CH == 'e') || (CUR_CH == 'E'))
       )
    {
        ++sr->pos;

        if (Streader_end_reached(sr))
        {
            Streader_set_error(sr, "No digits found after exponent indicator");
            return false;
        }

        if (CUR_CH == '+')
        {
            ++sr->pos;
        }
        else if (CUR_CH == '-')
        {
            exponent_is_negative = true;
            ++sr->pos;
        }

        while (!Streader_end_reached(sr) && isdigit(CUR_CH))
        {
            assert(exponent < INT_MAX / 10);
            exponent *= 10;
            exponent += (int)(CUR_CH - '0');

            ++sr->pos;
        }

        // Require at least one digit
        if (!isdigit(sr->str[sr->pos - 1]))
        {
            Streader_set_error(sr, "No digits found after exponent indicator");
            return false;
        }
    }

    if (!Streader_end_reached(sr) && isalpha(CUR_CH))
    {
        Streader_set_error(sr, "Trailing letters after a number");
        return false;
    }

    // Optimise trailing zeros away from the significand
    if (significand != 0)
    {
        while (significand % 10 == 0)
        {
            significand /= 10;
            ++significand_shift;
        }
    }

    // Convert the number
    if (exponent_is_negative)
        exponent = -exponent;
    int final_shift = significand_shift + exponent;

    double result = (double)significand;

    double abs_magnitude = pow(10, abs(final_shift));
    if (final_shift >= 0)
        result *= abs_magnitude;
    else
        result /= abs_magnitude;

    if (is_negative)
        result = -result;

    if (dest != NULL)
        *dest = result;

    return true;
}

#undef SIGNIFICANT_MAX


bool Streader_read_string(Streader* sr, int64_t max_bytes, char* dest)
{
    assert(sr != NULL);
    assert(max_bytes >= 0);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    if (Streader_end_reached(sr))
    {
        Streader_set_error(sr, "Unexpected end of data");
        return false;
    }

    // Check opening double quote
    if (CUR_CH != '\"')
    {
        if (isprint(CUR_CH))
            Streader_set_error(
                    sr,
                    "Expected beginning of a string instead of '%c'",
                    CUR_CH);
        else
            Streader_set_error(
                    sr,
                    "Expected beginning of a string instead of %#2x",
                    (unsigned)CUR_CH);
        return false;
    }
    ++sr->pos;

    // Parse and copy the string
    int64_t write_pos = 0;
    while (!Streader_end_reached(sr) && CUR_CH != '\"')
    {
        if (CUR_CH == '\\')
        {
            // Escape sequences
            ++sr->pos;
            if (Streader_end_reached(sr))
            {
                Streader_set_error(sr, "Unexpected end of data");
                return false;
            }

            char special_ch = '\0';
            if (strchr("\"\\/", CUR_CH) != NULL)
                special_ch = CUR_CH;
            else if (CUR_CH == 'b')
                special_ch = '\b';
            else if (CUR_CH == 'f')
                special_ch = '\f';
            else if (CUR_CH == 'n')
                special_ch = '\n';
            else if (CUR_CH == 'r')
                special_ch = '\r';
            else if (CUR_CH == 't')
                special_ch = '\t';
            else if (CUR_CH == 'u')
            {
                static const char* upper_hex_digits = "ABCDEF";
                static const char* lower_hex_digits = "abcdef";

                int32_t code = 0;

                ++sr->pos;
                for (int i = 0; i < 4; ++i)
                {
                    if (Streader_end_reached(sr))
                    {
                        Streader_set_error(sr, "Unexpected end of data");
                        return false;
                    }

                    int32_t value = -1;
                    if (isdigit(CUR_CH))
                        value = CUR_CH - '0';
                    else if (strchr(upper_hex_digits, CUR_CH) != NULL)
                        value = (int32_t)(strchr(upper_hex_digits, CUR_CH) -
                            upper_hex_digits + 0xa);
                    else if (strchr(lower_hex_digits, CUR_CH) != NULL)
                        value = (int32_t)(strchr(lower_hex_digits, CUR_CH) -
                            lower_hex_digits + 0xa);
                    else
                    {
                        char wrong_char[5] = "";
                        print_wrong_char(wrong_char, CUR_CH);
                        Streader_set_error(
                                sr,
                                "Expected a hexadecimal digit instead of %s",
                                wrong_char);
                        return false;
                    }

                    assert(value >= 0);
                    assert(value < 0x10);

                    code *= 0x10;
                    code += value;

                    ++sr->pos;
                }

                if (code < 0x20 || code > 0x7e)
                {
                    Streader_set_error(
                            sr,
                            "Unicode character U+%04X outside permitted"
                                " range [U+0020,U+007E]",
                            code);
                    return false;
                }

                special_ch = (char)code;
            }
            else
            {
                if (isprint(CUR_CH))
                    Streader_set_error(
                            sr, "Invalid escape sequence '\\%c'", CUR_CH);
                else
                    Streader_set_error(
                            sr, "Invalid escape sequence %#2x", CUR_CH);
                return false;
            }

            if (special_ch < 0x20 || special_ch > 0x7e)
            {
                char wrong_char[5] = "";
                print_wrong_char(wrong_char, special_ch);
                Streader_set_error(sr, "Invalid special character %s", wrong_char);
                return false;
            }

            if (dest != NULL && write_pos < max_bytes - 1)
                dest[write_pos++] = special_ch;
        }
        else if (iscntrl(CUR_CH))
        {
            Streader_set_error(sr, "Control character %#2x in string");
            return false;
        }
        else
        {
            // Normal characters
            // TODO: check Unicode if needed
            if (dest != NULL && write_pos + 1 < max_bytes)
                dest[write_pos++] = CUR_CH;
        }
        ++sr->pos;
    }

    // Check closing double quote
    if (Streader_end_reached(sr))
    {
        Streader_set_error(sr, "Unexpected end of data");
        return false;
    }
    assert(CUR_CH == '\"');
    ++sr->pos;

    if (max_bytes > 0 && dest != NULL)
    {
        assert(write_pos < max_bytes);
        dest[write_pos] = '\0';
    }

    return true;
}


bool Streader_read_tstamp(Streader* sr, Tstamp* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    int64_t beats = 0;
    int64_t rem = 0;

    if (!(Streader_match_char(sr, '[')  &&
                Streader_read_int(sr, &beats) &&
                Streader_match_char(sr, ',')  &&
                Streader_read_int(sr, &rem)   &&
                Streader_match_char(sr, ']')))
    {
        Streader_set_error(sr, "Expected a valid timestamp");
        return false;
    }

    if (rem < 0 || rem >= KQT_TSTAMP_BEAT)
    {
        Streader_set_error(
                sr,
                "Timestamp remainder %" PRId64 " out of range [0..%ld)",
                rem,
                KQT_TSTAMP_BEAT);
        return false;
    }

    if (dest != NULL)
        Tstamp_set(dest, beats, (int32_t)rem);

    return true;
}


static void recover(Streader* sr, int64_t pos)
{
    assert(sr != NULL);
    assert(pos <= sr->pos);

    Streader_clear_error(sr);
    sr->pos = pos;

    return;
}


bool Streader_read_finite_rt(Streader* sr, Value* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);
    const int64_t start_pos = sr->pos;

    Value* value = VALUE_AUTO;

    if (Streader_read_bool(sr, &value->value.bool_type))
        value->type = VALUE_TYPE_BOOL;
    else if ((recover(sr, start_pos), Streader_read_int(sr, &value->value.int_type)) &&
            (CUR_CH != '.') && (CUR_CH != 'e') && (CUR_CH != 'E'))
        value->type = VALUE_TYPE_INT;
    else if (recover(sr, start_pos),
            (Streader_read_float(sr, &value->value.float_type) &&
             isfinite(value->value.float_type)))
        value->type = VALUE_TYPE_FLOAT;
    else if (recover(sr, start_pos), Streader_read_tstamp(sr, &value->value.Tstamp_type))
        value->type = VALUE_TYPE_TSTAMP;

    if (value->type == VALUE_TYPE_NONE)
        return false;

    if (dest != NULL)
        Value_copy(dest, value);

    return true;
}


bool Streader_read_piref(Streader* sr, Pat_inst_ref* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    int64_t pat = 0;
    int64_t inst = 0;

    if (!(Streader_match_char(sr, '[') &&
                Streader_read_int(sr, &pat)  &&
                Streader_match_char(sr, ',') &&
                Streader_read_int(sr, &inst) &&
                Streader_match_char(sr, ']')))
    {
        Streader_set_error(sr, "Expected a valid pattern instance");
        return false;
    }

    if (pat < 0 || pat >= KQT_PATTERNS_MAX)
    {
        Streader_set_error(
                sr,
                "Pattern number %" PRId64 " out of range [0..%ld)"
                    " in pattern instance",
                pat,
                KQT_PATTERNS_MAX);
        return false;
    }

    if (inst < 0 || inst >= KQT_PAT_INSTANCES_MAX)
    {
        Streader_set_error(
                sr,
                "Pattern instance number %" PRId64 " out of range [0..%ld)"
                    " in pattern instance",
                inst,
                KQT_PAT_INSTANCES_MAX);
        return false;
    }

    if (dest != NULL)
    {
        dest->pat = (int16_t)pat;
        dest->inst = (int16_t)inst;
    }

    return true;
}


bool Streader_read_list(Streader* sr, List_item_reader ir, void* userdata)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    // Check opening bracket
    if (!(Streader_skip_whitespace(sr) && Streader_match_char(sr, '[')))
    {
        Streader_set_error(sr, "Expected a list opening bracket");
        return false;
    }

    // Check empty list
    if (Streader_try_match_char(sr, ']'))
        return true;

    if (ir == NULL)
    {
        Streader_set_error(sr, "Expected an empty list");
        return false;
    }

    // Read elements
    int32_t index = 0;
    do
    {
        if (!ir(sr, index, userdata))
            return false;
        ++index;
    } while (Streader_try_match_char(sr, ','));

    // Check closing bracket
    if (!Streader_match_char(sr, ']'))
    {
        Streader_set_error(sr, "Expected a list closing bracket");
        return false;
    }

    return true;
}


bool Streader_read_dict(Streader* sr, Dict_item_reader ir, void* userdata)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    // Check opening brace
    if (!(Streader_skip_whitespace(sr) && Streader_match_char(sr, '{')))
    {
        Streader_set_error(sr, "Expected a dictionary opening brace");
        return false;
    }

    // Check empty dictionary
    if (Streader_try_match_char(sr, '}'))
        return true;

    if (ir == NULL)
    {
        Streader_set_error(sr, "Expected an empty dictionary");
        return false;
    }

    // Read key/value pairs
    do
    {
        char key[STREADER_DICT_KEY_LENGTH_MAX + 1] = "";
        if (!(Streader_read_string(sr, STREADER_DICT_KEY_LENGTH_MAX + 1, key) &&
                    Streader_match_char(sr, ':') &&
                    ir(sr, key, userdata)))
            return false;
    } while (Streader_try_match_char(sr, ','));

    // Check closing brace
    if (!Streader_match_char(sr, '}'))
    {
        Streader_set_error(sr, "Expected a dictionary closing brace");
        return false;
    }

    return true;
}


bool Streader_readf(Streader* sr, const char* format, ...)
{
    assert(sr != NULL);
    assert(format != NULL);

    va_list args;
    va_start(args, format);

    while (!Streader_is_error_set(sr) && *format != '\0')
    {
        if (*format == '%')
        {
            // Conversion characters
            ++format;

            switch (*format)
            {
                case 'n':
                {
                    Streader_read_null(sr);
                }
                break;

                case 'b':
                {
                    bool* dest = va_arg(args, bool*);
                    Streader_read_bool(sr, dest);
                }
                break;

                case 'i':
                {
                    int64_t* dest = va_arg(args, int64_t*);
                    Streader_read_int(sr, dest);
                }
                break;

                case 'f':
                {
                    double* dest = va_arg(args, double*);
                    Streader_read_float(sr, dest);
                }
                break;

                case 's':
                {
                    size_t max_bytes = va_arg(args, size_t);
                    char* dest = va_arg(args, char*);
                    Streader_read_string(sr, (int64_t)max_bytes, dest);
                }
                break;

                case 't':
                {
                    Tstamp* dest = va_arg(args, Tstamp*);
                    Streader_read_tstamp(sr, dest);
                }
                break;

                case 'p':
                {
                    Pat_inst_ref* dest = va_arg(args, Pat_inst_ref*);
                    Streader_read_piref(sr, dest);
                }
                break;

                case 'l':
                {
                    List_item_reader* ir = va_arg(args, List_item_reader*);
                    void* userdata = va_arg(args, void*);
                    Streader_read_list(sr, ir, userdata);
                }
                break;

                case 'd':
                {
                    Dict_item_reader* ir = va_arg(args, Dict_item_reader*);
                    void* userdata = va_arg(args, void*);
                    Streader_read_dict(sr, ir, userdata);
                }
                break;

                case '%':
                {
                    Streader_match_char(sr, '%');
                }
                break;

                default:
                    assert(false);
            }
        }
        else
        {
            // Characters to be matched
            if (!isspace(*format))
                Streader_match_char(sr, *format);
        }

        ++format;
    }

    va_end(args);

    return !Streader_is_error_set(sr);
}


