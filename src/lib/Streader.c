

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <kunquat/limits.h>
#include <Streader.h>
#include <xassert.h>


Streader* Streader_init(Streader* sr, const char* str, size_t len)
{
    assert(sr != NULL);
    assert(str != NULL || len == 0);

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


void Streader_clear_error(Streader* sr)
{
    assert(sr != NULL);
    Error_clear(&sr->error);
    return;
}


static bool Streader_end_reached(Streader* sr)
{
    assert(sr != NULL);
    assert(sr->pos <= sr->len);

    return sr->pos == sr->len;
}


#define CUR_CH (assert(!Streader_end_reached(sr)), sr->str[sr->pos])


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

    const size_t start_pos = sr->pos;
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
    const size_t expected_len = strlen(seq);
    if (sr->len - sr->pos < expected_len)
    {
        Streader_set_error(
                sr,
                "Too few bytes left to contain expected string `%s`",
                seq);
        return false;
    }

    // Match the string
    if (strncmp(&sr->str[sr->pos], seq, expected_len) != 0)
    {
        Streader_set_error(sr, "Unexpected string (expected `%s`)", seq);
        return false;
    }

    // Update position
    for (size_t i = 0; i < expected_len; ++i)
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

    const size_t start_pos = sr->pos;
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


#define DECIMAL_CHARS_MAX 256

#define CHECK_SPACE(err_msg)                   \
    if (true)                                  \
    {                                          \
        if (write_pos > DECIMAL_CHARS_MAX)     \
        {                                      \
            Streader_set_error(sr, (err_msg)); \
            return false;                      \
        }                                      \
    } else (void)0

bool Streader_read_float(Streader* sr, double* dest)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_skip_whitespace(sr);

    // Just copy legal characters to our buffer and
    // let strtod handle the rest for now. TODO: revisit

    char num_chars[DECIMAL_CHARS_MAX + 1] = "";
    int write_pos = 0;

    static const char* len_err_msg = "Number representation is too long";

    // Negation
    if (Streader_try_match_char(sr, '-'))
        num_chars[write_pos++] = '-';

    if (Streader_end_reached(sr))
    {
        Streader_set_error(sr, "No digits found");
        return false;
    }

    // Significand
    if (Streader_try_match_char_seq(sr, "0"))
    {
        num_chars[write_pos++] = '0';
    }
    else if (strchr(NONZERO_DIGITS, CUR_CH) != NULL)
    {
        while (!Streader_end_reached(sr) && isdigit(CUR_CH))
        {
            CHECK_SPACE("Too many digits in the significand");
            num_chars[write_pos++] = CUR_CH;
            ++sr->pos;
        }
    }
    else
    {
        Streader_set_error(sr, "No digits found");
        return false;
    }

    // Decimal part
    if (!Streader_end_reached(sr) && Streader_try_match_char_seq(sr, "."))
    {
        CHECK_SPACE(len_err_msg);
        num_chars[write_pos++] = '.';

        while (!Streader_end_reached(sr) && isdigit(CUR_CH))
        {
            CHECK_SPACE(len_err_msg);
            num_chars[write_pos++] = CUR_CH;
            ++sr->pos;
        }

        // Require at least one digit
        if (num_chars[write_pos - 1] == '.')
        {
            Streader_set_error(sr, "No digits found after decimal point");
            return false;
        }
    }

    // Exponent part
    if (!Streader_end_reached(sr) &&
            (Streader_try_match_char_seq(sr, "e") ||
             Streader_try_match_char_seq(sr, "E"))
       )
    {
        CHECK_SPACE("Number representation is too long");
        num_chars[write_pos++] = 'e';

        if (Streader_end_reached(sr))
        {
            Streader_set_error(sr, "No digits found after exponent indicator");
            return false;
        }

        if (Streader_try_match_char_seq(sr, "+"))
        {
        }
        else if (Streader_try_match_char_seq(sr, "-"))
        {
            CHECK_SPACE(len_err_msg);
            num_chars[write_pos++] = '-';
        }

        while (!Streader_end_reached(sr) && isdigit(CUR_CH))
        {
            CHECK_SPACE(len_err_msg);
            num_chars[write_pos++] = CUR_CH;
            ++sr->pos;
        }

        // Require at least one digit
        if (!isdigit(num_chars[write_pos - 1]))
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

    // Convert the number
    char* end = NULL;
    double result = strtod(num_chars, &end);
    if (end == num_chars || !isfinite(result))
    {
        Streader_set_error(sr, "Floating-point number is not valid");
        return false;
    }

    if (dest != NULL)
        *dest = result;

    return true;
}


bool Streader_read_string(Streader* sr, size_t max_bytes, char* dest)
{
    assert(sr != NULL);

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
    size_t write_pos = 0;
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
                Streader_set_error(
                        sr, "Escaped hex digit sequences are not supported");
                return false;
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

            if (dest != NULL && write_pos + 1 < max_bytes)
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
        Tstamp_set(dest, beats, rem);

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
        dest->pat = pat;
        dest->inst = inst;
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
                    Streader_read_string(sr, max_bytes, dest);
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


