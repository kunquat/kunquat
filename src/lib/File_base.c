

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include <Real.h>
#include <Reltime.h>
#include <File_base.h>

#include <xmemory.h>


#define return_null_if(cond, state, msg)                                \
    do                                                                  \
    {                                                                   \
        if ((cond))                                                     \
        {                                                               \
            strncpy((state)->message, (msg), ERROR_MESSAGE_LENGTH - 1); \
            (state)->message[ERROR_MESSAGE_LENGTH - 1] = '\0';          \
            return NULL;                                                \
        }                                                               \
    } while (false)


char* read_file(FILE* in, Read_state* state)
{
    assert(in != NULL);
    assert(state != NULL);
    
    errno = 0;
    int err = fseek(in, 0, SEEK_END);
    return_null_if(err < 0, state, strerror(errno));
    
    errno = 0;
    long length = ftell(in);
    return_null_if(length < 0, state, strerror(errno));
    
    errno = 0;
    err = fseek(in, 0, SEEK_SET);
    return_null_if(err < 0, state, strerror(errno));
    
    char* data = xcalloc(char, length + 1);
    return_null_if(data == NULL, state, "Couldn't allocate memory for input file.");
    long pos = 0;
    char* location = data;
    while (pos < length)
    {
        size_t read = fread(location, 1, 1024, in);
        pos += 1024;
        location += 1024;
        if (read < 1024 && pos < length)
        {
            strncpy(state->message, "Couldn't read data from the input file",
                    ERROR_MESSAGE_LENGTH - 1);
            state->message[ERROR_MESSAGE_LENGTH - 1] = '\0';
            xfree(data);
            return NULL;
        }
    }
    return data;
}


#undef return_null_if


char* skip_line(char* str, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    while (*str != '\0' && *str != '\n' && *str != '\r')
    {
        ++str;
    }
    ++state->row;
    return str;
}


char* skip_whitespace(char* str, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    while (isspace(*str))
    {
        if (*str == '\n')
        {
            ++state->row;
        }
        ++str;
    }
    return str;
}


char* read_const_char(char* str, char result, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = skip_whitespace(str, state);
    if (*str != result)
    {
        state->error = true;
        if (isprint(*str))
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected '%c' instead of '%c'", result, *str);
        }
        else
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected '%c' instead of %#2x", result, (unsigned)*str);
        }
        return str;
    }
    if (*str != '\0')
    {
        ++str;
    }
    return str;
}


char* read_const_string(char* str, char* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = skip_whitespace(str, state);
    if (*str != '\"')
    {
        state->error = true;
        if (isprint(*str))
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected beginning of a string instead of '%c'", *str);
        }
        else
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected beginning of a string instead of %#2x", (unsigned)*str);
        }
        return str;
    }
    ++str;
    int len = strlen(result);
    if (strncmp(str, result, len) != 0)
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Unexpected string (expected \"%s\")", result);
        return str;
    }
    for (int i = 0; i < len; ++i)
    {
        assert(*str != '\0');
        if (*str == '\n')
        {
            ++state->row;
        }
        ++str;
    }
    if (*str != '\"')
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Unexpected string (expected \"%s\")", result);
        return str;
    }
    ++str;
    return str;
}


char* read_string(char* str, char* result, int max_len, Read_state* state)
{
    assert(str != NULL);
    assert((result != NULL) || (max_len <= 0));
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = skip_whitespace(str, state);
    if (*str != '\"')
    {
        state->error = true;
        if (isprint(*str))
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected beginning of a string instead of '%c'", *str);
        }
        else
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected beginning of a string instead of %#2x", (unsigned)*str);
        }
        return str;
    }
    ++str;
    for (int i = 0; i < max_len - 1; ++i)
    {
        assert(result != NULL);
        if (*str == '\0')
        {
            state->error = true;
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Unexpected end of string");
            return str;
        }
        if (*str == '\"')
        {
            break;
        }
        else if (*str == '\\')
        {
            if (i == max_len - 2)
            {
                break;
            }
            ++str;
            if (strchr("\"\\/", *str) != NULL)
            {
                *result = *str;
            }
            else if (*str == 'n')
            {
                *result = '\n';
            }
            else if (*str == 't')
            {
                *result = '\t';
            }
            else if (strchr("\b\r\f", *str) == NULL)
            {
                state->error = true;
                snprintf(state->message, ERROR_MESSAGE_LENGTH,
                        "Unsupported escape sequence '\\%c'", *str);
                return str;
            }
            ++result;
            ++str;
            continue;
        }
        *result++ = *str++;
    }
    if (max_len > 0)
    {
        assert(result != NULL);
        *result = '\0';
    }
    while (*str != '\"')
    {
        if (*str == '\0')
        {
            state->error = true;
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Unexpected end of string");
            return str;
        }
        ++str;
    }
    ++str;
    return str;
}


char* read_int(char* str, int64_t* result, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    char* end = NULL;
    str = skip_whitespace(str, state);
    int64_t val = strtol(str, &end, 0);
    if (str == end)
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Expected a valid integer");
        return str;
    }
    if (result != NULL)
    {
        *result = val;
    }
    return end;
}


char* read_double(char* str, double* result, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    char* end = NULL;
    str = skip_whitespace(str, state);
    double val = strtod(str, &end);
    if (end == str)
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Expected a valid floating point number");
        return str;
    }
    if (result != NULL)
    {
        *result = val;
    }
    return end;
}


char* read_tuning(char* str, Real* result, double* cents, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        return str;
    }
    char type[2] = { '\0' };
    str = read_string(str, type, 2, state);
    if (state->error)
    {
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Expected a type description of the ratio");
        return str;
    }
    str = read_const_char(str, ',', state);
    if (state->error)
    {
        return str;
    }
    int64_t num = 0;
    int64_t den = 0;
    double fl = 0;
    if (type[0] == '/')
    {
        str = read_int(str, &num, state);
        if (state->error)
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected a numerator for a fraction");
            return str;
        }
        str = read_const_char(str, ',', state);
        if (state->error)
        {
            return str;
        }
        str = read_int(str, &den, state);
        if (state->error)
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Expected a denominator for a fraction");
            return str;
        }
        if (den <= 0)
        {
            state->error = true;
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Denominator must be positive");
            return str;
        }
    }
    else if (type[0] == 'f' || type[0] == 'c')
    {
        str = read_double(str, &fl, state);
        if (state->error)
        {
            return str;
        }
    }
    else
    {
        state->error = true;
        if (isprint(type[0]))
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Invalid type description '%c'", type[0]);
        }
        else
        {
            snprintf(state->message, ERROR_MESSAGE_LENGTH,
                    "Invalid type description (%#2x)", type[0]);
        }
        return str;
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        return str;
    }
    if (type[0] == '/')
    {
        if (result != NULL)
        {
            Real_init_as_frac(result, num, den);
        }
        if (cents != NULL)
        {
            *cents = NAN;
        }
    }
    else if (type[0] == 'f')
    {
        if (result != NULL)
        {
            Real_init_as_double(result, fl);
        }
        if (cents != NULL)
        {
            *cents = NAN;
        }
    }
    else
    {
        assert(type[0] == 'c');
        if (cents != NULL)
        {
            *cents = fl;
        }
    }
    return str;
}


char* read_reltime(char* str, Reltime* result, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    int64_t beats = 0;
    int64_t rem = 0;
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        return str;
    }
    str = read_int(str, &beats, state);
    if (state->error)
    {
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Expected a valid Reltime stamp");
        return str;
    }
    str = read_const_char(str, ',', state);
    if (state->error)
    {
        return str;
    }
    ++str;
    str = read_int(str, &rem, state);
    if (state->error)
    {
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Expected a valid Reltime stamp");
        return str;
    }
    if (rem < 0 || rem > RELTIME_BEAT)
    {
        state->error = true;
        snprintf(state->message, ERROR_MESSAGE_LENGTH,
                "Reltime stamp remainder out of range [0..%ld)", RELTIME_BEAT);
        return str;
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        return str;
    }
    if (result != NULL)
    {
        Reltime_set(result, beats, rem);
    }
    return str;
}


