

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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


#include <stdlib.h>


/**
 * Suppresses error message printing.
 */
void xassert_suppress_messages(void);


#ifdef ENABLE_KUNQUAT_ASSERT


/**
 * This is an internal implementation of assert.
 */


#if defined(HAS_EXECINFO) && !defined(SILENT_ASSERT)

/**
 * Prints a backtrace of the execution.
 */
void xassert_print_backtrace(void);

#else // !HAS_EXECINFO || SILENT_ASSERT

#define xassert_print_backtrace() (void)0

#endif // !HAS_EXECINFO || SILENT_ASSERT


#ifndef SILENT_ASSERT

/**
 * Prints a message of failed assertion.
 *
 * \param file_name     The source code file name -- must not be \c NULL.
 * \param line_number   The source code file line number.
 * \param func_name     The name of the function -- must not be \c NULL.
 * \param assertion     The assertion that failed -- must not be \c NULL.
 */
void xassert_print_msg(
        const char* file_name,
        int line_number,
        const char* func_name,
        const char* assertion);

#else // SILENT_ASSERT

#define xassert_print_msg() (void)0

#endif // SILENT_ASSERT


#ifndef NDEBUG

#define assert(expr)                                                \
    (                                                               \
        (expr) ?                                                    \
            (void)0                                                 \
        :                                                           \
        (                                                           \
            xassert_print_msg(__FILE__, __LINE__, __func__, #expr), \
            xassert_print_backtrace(),                              \
            abort()                                                 \
        )                                                           \
    )

#else // NDEBUG

#define assert(expr) (void)0

#endif // NDEBUG


#else // ENABLE_KUNQUAT_ASSERT

#include <assert.h>

#endif // ENABLE_KUNQUAT_ASSERT


#endif // K_XASSERT_H


