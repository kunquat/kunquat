

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Error.h>

#include <debug/assert.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const char* error_codes[ERROR_COUNT_] =
{
    [ERROR_ARGUMENT] = "ArgumentError",
    [ERROR_FORMAT] = "FormatError",
    [ERROR_MEMORY] = "MemoryError",
    [ERROR_RESOURCE] = "ResourceError",
};


bool Error_is_set(const Error* error)
{
    rassert(error != NULL);
    return error->desc[0] != '\0';
}


Error_type Error_get_type(const Error* error)
{
    rassert(error != NULL);
    rassert(Error_is_set(error));
    rassert(error->type < ERROR_COUNT_);

    return error->type;
}


const char* Error_get_desc(const Error* error)
{
    rassert(error != NULL);
    return error->desc;
}


const char* Error_get_message(const Error* error)
{
    rassert(error != NULL);
    return error->message;
}


void Error_copy(Error* restrict dest, const Error* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(src != dest);

    memcpy(dest, src, sizeof(Error));

    return;
}


void Error_set_desc(
        Error* error,
        Error_type type,
        const char* file,
        int line,
        const char* func,
        const char* message,
        ...)
{
    rassert(error != NULL);
    rassert(type < ERROR_COUNT_);
    rassert(file != NULL);
    rassert(line >= 0);
    rassert(func != NULL);
    rassert(message != NULL);

    va_list args;
    va_start(args, message);
    Error_set_desc_va_list(error, type, file, line, func, message, args);
    va_end(args);

    return;
}


void Error_set_desc_va_list(
        Error* error,
        Error_type type,
        const char* file,
        int line,
        const char* func,
        const char* message,
        va_list args)
{
    rassert(error != NULL);
    rassert(type < ERROR_COUNT_);
    rassert(file != NULL);
    rassert(line >= 0);
    rassert(func != NULL);
    rassert(message != NULL);

    Error_clear(error);

    strcpy(error->desc, "{ \"type\": \"");
    strcat(error->desc, error_codes[type]);
    strcat(error->desc, "\", ");

    strcat(error->desc, "\"file\": \"");
    strcat(error->desc, file);
    strcat(error->desc, "\", ");

    sprintf(&error->desc[strlen(error->desc)], "\"line\": %d, ", line);

    strcat(error->desc, "\"function\": \"");
    strcat(error->desc, func);
    strcat(error->desc, "\", ");

    strcat(error->desc, "\"message\": \"");
    vsnprintf(error->message, ERROR_LENGTH_MAX, message, args);
    size_t json_pos = strlen(error->desc);

    for (int i = 0;
            json_pos < ERROR_LENGTH_MAX - 4 && error->message[i] != '\0';
            ++i, ++json_pos)
    {
        char ch = error->message[i];
        static const char named_controls[] = "\"\\\b\f\n\r\t";
        static const char* named_replace[] =
                { "\\\"", "\\\\", "\\b", "\\f", "\\n", "\\r", "\\t" };
        const char* named_control = strchr(named_controls, ch);
        if (named_control != NULL)
        {
            if (json_pos >= ERROR_LENGTH_MAX - 5)
                break;

            const ptrdiff_t pos = named_control - named_controls;
            rassert(pos >= 0);
            rassert(pos < (int)strlen(named_controls));
            strcpy(&error->desc[json_pos], named_replace[pos]);
            json_pos += strlen(named_replace[pos]) - 1;
        }
        else if (ch < 0x20 || ch == 0x7f)
        {
            if (json_pos >= ERROR_LENGTH_MAX - 4 - 5)
                break;

            // FIXME: We should really check for all control characters
            char code[] = "\\u0000";
            snprintf(code, strlen(code) + 1, "\\u%04x", ch);
            strcpy(&error->desc[json_pos], code);
            json_pos += strlen(code) - 1;
        }
        else
        {
            error->desc[json_pos] = ch;
        }
    }

    strcat(error->desc, "\" }");
    error->desc[ERROR_LENGTH_MAX - 1] = '\0';

    error->type = type;

    return;
}


void Error_clear(Error* error)
{
    rassert(error != NULL);

    memset(error->desc, 0, ERROR_LENGTH_MAX);
    memset(error->message, 0, ERROR_LENGTH_MAX);

    return;
}


