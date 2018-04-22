

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


#ifndef KQT_ERROR_H
#define KQT_ERROR_H


#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>


#define ERROR_LENGTH_MAX 512


typedef enum
{
    ERROR_ARGUMENT, // libkunquat function called with an invalid argument
    ERROR_FORMAT,   // input file structure error
    ERROR_MEMORY,   // out of memory
    ERROR_RESOURCE, // resource (external lib or fs) failure
    ERROR_COUNT_
} Error_type;


/**
 * A container for an error/warning description.
 */
typedef struct Error
{
    char desc[ERROR_LENGTH_MAX];
    char message[ERROR_LENGTH_MAX];
    Error_type type;
} Error;


#define ERROR_AUTO (&(Error){ .desc = "", .message = "", .type = ERROR_COUNT_ })


/**
 * Return true if an error is set.
 *
 * \param error   The Error -- must not be \c NULL.
 *
 * \return   \c true if an error is set, otherwise \c false.
 */
bool Error_is_set(const Error* error);


/**
 * Get the type of the error set.
 *
 * \param error   The Error -- must not be \c NULL and must have an error set.
 *
 * \return   The error type.
 */
Error_type Error_get_type(const Error* error);


/**
 * Get an error description.
 *
 * \param error   The Error -- must not be \c NULL.
 *
 * \return   The error description in JSON format, or an empty string.
 */
const char* Error_get_desc(const Error* error);


/**
 * Get a human-readable error message.
 *
 * \param error   The Error -- must not be \c NULL.
 *
 * \return   The human-readable error message, or an empty string.
 */
const char* Error_get_message(const Error* error);


/**
 * Make a copy of an Error.
 *
 * \param dest   The destination Error -- must not be \c NULL.
 * \param src    The source Error -- must not be \c NULL or \a dest.
 */
void Error_copy(Error* restrict dest, const Error* restrict src);


/**
 * Set an error description.
 */
#define Error_set(error, type, ...) \
    (Error_set_desc((error), (type), __FILE__, __LINE__, __func__, __VA_ARGS__))

void Error_set_desc(
        Error* error,
        Error_type type,
        const char* file,
        int line,
        const char* func,
        const char* message,
        ...);


/**
 * Set an error description.
 *
 * \param error     The Error -- must not be \c NULL.
 * \param type      The error type -- must be < ERROR_COUNT_.
 * \param file      The source file name -- must not be \c NULL.
 * \param line      The line of the source file -- must not be negative.
 * \param func      The function name -- must not be \c NULL.
 * \param message   A human-readable message format -- must not be \c NULL.
 * \param args      Argument list -- must be valid.
 */
void Error_set_desc_va_list(
        Error* error,
        Error_type type,
        const char* file,
        int line,
        const char* func,
        const char* message,
        va_list args);


/**
 * Clear an Error.
 *
 * \param error   The Error -- must not be \c NULL.
 */
void Error_clear(Error* error);


#endif // KQT_ERROR_H


