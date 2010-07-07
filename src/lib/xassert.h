

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


#ifndef K_XASSERT_H
#define K_XASSERT_H


#ifdef ENABLE_KUNQUAT_ASSERT


/**
 * This is an internal implementation of assert. Note that this version of
 * assert can only be used as a statement, not as an expression.
 */


#include <stdbool.h>
#include <stdio.h>


#if defined(HAS_EXECINFO) && !defined(SILENT_ASSERT)

#include <execinfo.h>

#define BACKTRACE_LEVELS_MAX 256

#define xassert_print_backtrace()                                   \
    if (true)                                                       \
    {                                                               \
        void* buffer[BACKTRACE_LEVELS_MAX] = { NULL };              \
        size_t size = backtrace(buffer, BACKTRACE_LEVELS_MAX);      \
        char** symbols = backtrace_symbols(buffer, size);           \
        if (symbols == NULL)                                        \
        {                                                           \
            fprintf(stderr, "(Could not allocate memory"            \
                            " for a backtrace)\n");                 \
        }                                                           \
        else                                                        \
        {                                                           \
            fprintf(stderr, "Backtrace:\n");                        \
            for (size_t i = 0; i < size; ++i)                       \
            {                                                       \
                fprintf(stderr, "  %s\n", symbols[i]);              \
            }                                                       \
            if (size >= BACKTRACE_LEVELS_MAX)                       \
            {                                                       \
                fprintf(stderr, "(Reached maximum number of levels" \
                                " in the backtrace)\n");            \
            }                                                       \
            free(symbols);                                          \
        }                                                           \
    } else (void)0

#else // !HAS_EXECINFO || SILENT_ASSERT

#define xassert_print_backtrace() (void)0

#endif // !HAS_EXECINFO || SILENT_ASSERT


#ifndef SILENT_ASSERT

#define xassert_print_msg(str)                              \
    if (true)                                               \
    {                                                       \
        fprintf(stderr, "libkunquat: %s:%d: %s: "           \
                        "Assertion `%s' failed.\n",         \
                        __FILE__, __LINE__, __func__, str); \
    } else (void)0

#else // SILENT_ASSERT

#define xassert_print_msg() (void)0

#endif // SILENT_ASSERT


#define assert(expr)                   \
    if (true)                          \
    {                                  \
        if (!(expr))                   \
        {                              \
            xassert_print_msg(#expr);  \
            xassert_print_backtrace(); \
            abort();                   \
        }                              \
    } else (void)0


#else // ENABLE_KUNQUAT_ASSERT

#include <assert.h>

#endif // ENABLE_KUNQUAT_ASSERT


#endif // K_XASSERT_H


