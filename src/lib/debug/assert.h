

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ASSERT_H
#define KQT_ASSERT_H


#include <common.h>

#include <stdlib.h>


/**
 * Suppress error message printing.
 */
void assert_suppress_messages(void);


/**
 * Static (compile-time) assert.
 */
#ifndef static_assert
#define concat_(x, y) x ## y
#define make_unique2_(x, y) concat_(x, y)
#define make_unique_(prefix) make_unique2_(prefix, __LINE__)
#define static_assert(expr, msg) \
    typedef char make_unique_(static_assert_) [(expr) ? 1 : -1]
#endif


#if defined(HAS_EXECINFO) && !defined(SILENT_ASSERT)
/**
 * Print a backtrace of the execution.
 */
void assert_print_backtrace(void);

#else // !HAS_EXECINFO || SILENT_ASSERT

#define assert_print_backtrace() ignore(0)

#endif // !HAS_EXECINFO || SILENT_ASSERT


#ifndef SILENT_ASSERT
/**
 * Print a message of failed assertion.
 *
 * \param file_name     The source code file name -- must not be \c NULL.
 * \param line_number   The source code file line number.
 * \param func_name     The name of the function -- must not be \c NULL.
 * \param assertion     The assertion that failed -- must not be \c NULL.
 */
void assert_print_msg(
        const char* file_name,
        int line_number,
        const char* func_name,
        const char* assertion);

#else // SILENT_ASSERT

#define assert_print_msg(file, line, func, expr) ignore(0)

#endif // SILENT_ASSERT


// Release assert that is always compiled and should be used in most cases.
#define rassert(expr)                                              \
    (                                                              \
        (expr) ?                                                   \
            ignore(0)                                              \
        :                                                          \
        (                                                          \
            assert_print_msg(__FILE__, __LINE__, __func__, #expr), \
            assert_print_backtrace(),                              \
            abort()                                                \
        )                                                          \
    )


// Debug assert that should only be used in performance-critical code.
#ifdef ENABLE_DEBUG_ASSERTS
#define dassert rassert
#else
#define dassert(expr) ignore(expr)
#endif


#endif // KQT_ASSERT_H


