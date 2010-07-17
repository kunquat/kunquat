

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <stdarg.h>

#include <Real.h>
#include <Reltime.h>
#include <File_base.h>
#include <Handle_private.h>
#include <xassert.h>
#include <xmemory.h>


Read_state* Read_state_init(Read_state* state, const char* path)
{
    assert(state != NULL);
    assert(path != NULL);
    Read_state_clear_error(state);
    strncpy(state->path, path, STATE_PATH_LENGTH - 1);
    state->path[STATE_PATH_LENGTH - 1] = '\0';
    state->row = 1;
    return state;
}


void Read_state_set_error(Read_state* state, const char* message, ...)
{
    assert(state != NULL);
    assert(message != NULL);
    state->error = true;
    va_list args;
    va_start(args, message);
    vsnprintf(state->message, ERROR_MESSAGE_LENGTH, message, args);
    va_end(args);
    state->message[ERROR_MESSAGE_LENGTH - 1] = '\0';
    return;
}


void Read_state_clear_error(Read_state* state)
{
    assert(state != NULL);
    state->error = false;
    memset(state->message, 0, ERROR_MESSAGE_LENGTH);
//    state->message[0] = state->message[ERROR_MESSAGE_LENGTH - 1] = '\0';
    return;
}


#if 0
#define return_null_if(cond, handle, msg)                              \
    if (true)                                                          \
    {                                                                  \
        if ((cond))                                                    \
        {                                                              \
            kqt_Handle_set_error((handle), "%s: %s", __func__, (msg)); \
            return NULL;                                               \
        }                                                              \
    } else (void)0

char* read_file(FILE* in, long* size, kqt_Handle* handle)
{
    assert(in != NULL);
    assert(size != NULL);
    
    errno = 0;
    int err = fseek(in, 0, SEEK_END);
    return_null_if(err < 0, handle, strerror(errno));
    
    errno = 0;
    long length = ftell(in);
    return_null_if(length < 0, handle, strerror(errno));
    
    errno = 0;
    err = fseek(in, 0, SEEK_SET);
    return_null_if(err < 0, handle, strerror(errno));
    
    char* data = xcalloc(char, length + 1);
    return_null_if(data == NULL, handle, "Couldn't allocate memory");
    long pos = 0;
    char* location = data;
    *size = 0;
    while (pos < length)
    {
        size_t read = fread(location, 1, 1024, in);
        *size += read;
        pos += 1024;
        location += 1024;
        if (read < 1024 && pos < length)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't read data from the"
                    " input file", __func__);
            xfree(data);
            return NULL;
        }
    }
    return data;
}

#undef return_null_if
#endif


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


char* read_null(char* str, Read_state* state)
{
    assert(str != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = read_const_char(str, 'n', state);
    str = read_const_char(str, 'u', state);
    str = read_const_char(str, 'l', state);
    str = read_const_char(str, 'l', state);
    if (state->error)
    {
        Read_state_set_error(state, "Expected null");
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
        if (isprint(*str))
        {
            Read_state_set_error(state, "Expected '%c' instead of '%c'",
                                 result, *str);
        }
        else
        {
            Read_state_set_error(state, "Expected '%c' instead of %#04x",
                                         result, (unsigned)*str);
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
        if (isprint(*str))
        {
            Read_state_set_error(state,
                   "Expected beginning of a string instead of '%c'", *str);
        }
        else
        {
            Read_state_set_error(state,
                   "Expected beginning of a string instead of %#2x", (unsigned)*str);
        }
        return str;
    }
    ++str;
    int len = strlen(result);
    if (strncmp(str, result, len) != 0)
    {
        Read_state_set_error(state, "Unexpected string (expected \"%s\")", result);
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
        Read_state_set_error(state, "Unexpected string (expected \"%s\")", result);
        return str;
    }
    ++str;
    return str;
}


char* read_bool(char* str, bool* result, Read_state* state)
{
    assert(str != NULL);
    assert(result != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return str;
    }
    str = skip_whitespace(str, state);
    if (       str[0] == 't'
            && str[1] == 'r'
            && str[2] == 'u'
            && str[3] == 'e')
    {
        str += 4;
        *result = true;
    }
    else if (  str[0] == 'f'
            && str[1] == 'a'
            && str[2] == 'l'
            && str[3] == 's'
            && str[4] == 'e')
    {
        str += 5;
        *result = false;
    }
    else
    {
        Read_state_set_error(state, "Expected a valid boolean");
    }
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
        if (isprint(*str))
        {
            Read_state_set_error(state,
                    "Expected beginning of a string instead of '%c'", *str);
        }
        else
        {
            Read_state_set_error(state,
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
            Read_state_set_error(state, "Unexpected end of string");
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
                Read_state_set_error(state,
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
            Read_state_set_error(state, "Unexpected end of string");
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
        Read_state_set_error(state, "Expected a valid integer");
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
        Read_state_set_error(state, "Expected a valid floating point number");
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
        Read_state_set_error(state, "Expected a type description of the ratio");
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
        str = read_const_char(str, '[', state);
        if (state->error)
        {
            return str;
        }
        str = read_int(str, &num, state);
        if (state->error)
        {
            Read_state_set_error(state, "Expected a numerator for a fraction");
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
            Read_state_set_error(state, "Expected a denominator for a fraction");
            return str;
        }
        if (den <= 0)
        {
            Read_state_set_error(state, "Denominator must be positive");
            return str;
        }
        str = read_const_char(str, ']', state);
        if (state->error)
        {
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
        if (!isfinite(fl))
        {
            Read_state_set_error(state, "Floating point value must be finite.");
            return str;
        }
    }
    else
    {
        if (isprint(type[0]))
        {
            Read_state_set_error(state, "Invalid type description '%c'", type[0]);
        }
        else
        {
            Read_state_set_error(state, "Invalid type description (%#2x)", type[0]);
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
        Read_state_set_error(state, "Expected a valid Reltime stamp");
        return str;
    }
    str = read_const_char(str, ',', state);
    if (state->error)
    {
        return str;
    }
    str = read_int(str, &rem, state);
    if (state->error)
    {
        Read_state_set_error(state, "Expected a valid Reltime stamp");
        return str;
    }
    if (rem < 0 || rem > KQT_RELTIME_BEAT)
    {
        Read_state_set_error(state,
                "Reltime stamp remainder out of range [0..%ld)", KQT_RELTIME_BEAT);
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


