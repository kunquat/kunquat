

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <debug/assert.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


static bool is_printing_enabled = true;


void assert_suppress_messages(void)
{
    is_printing_enabled = false;
    return;
}


#if defined(HAS_EXECINFO) && !defined(SILENT_ASSERT)

#include <execinfo.h>

#define BACKTRACE_LEVELS_MAX 256

void assert_print_backtrace(void)
{
    if (!is_printing_enabled)
        return;

    void* buffer[BACKTRACE_LEVELS_MAX + 1] = { NULL };
    const int size = backtrace(buffer, BACKTRACE_LEVELS_MAX + 1);
    char** symbols = backtrace_symbols(buffer, size);
    if (symbols == NULL)
    {
        fprintf(stderr, "(Could not allocate memory for a backtrace)\n");
    }
    else
    {
        fprintf(stderr, "Backtrace:\n");

        for (int i = 1; i < size; ++i)
            fprintf(stderr, "  %s\n", symbols[i]);

        if (size >= BACKTRACE_LEVELS_MAX + 1)
        {
            fprintf(stderr,
                    "(Reached maximum number of levels in the backtrace)\n");
        }

        free(symbols);
    }
    return;
}

#endif // HAS_EXECINFO && !SILENT_ASSERT


#ifndef SILENT_ASSERT

void assert_print_msg(
        const char* file_name,
        int line_number,
        const char* func_name,
        const char* assertion)
{
    if (!is_printing_enabled)
        return;

    if (file_name == NULL || func_name == NULL || assertion == NULL)
    {
        fprintf(stderr, "(assert_print_msg called with illegal arguments)\n");
        return;
    }

    fprintf(stderr,
            "libkunquat: %s:%d: %s: Assertion `%s' failed.\n",
            file_name, line_number, func_name, assertion);

    return;
}

#endif // SILENT_ASSERT


