

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


#ifndef K_FILE_BASE_H
#define K_FILE_BASE_H


#include <stdbool.h>
#include <stdint.h>

#include <Real.h>
#include <Reltime.h>
#include <kunquat/Handle.h>


#define STATE_PATH_LENGTH (512)
#define ERROR_MESSAGE_LENGTH (256)

#define MAGIC_ID "kunquat"


typedef struct Read_state
{
    bool error;
    char path[STATE_PATH_LENGTH];
    int row;
    char message[ERROR_MESSAGE_LENGTH];
} Read_state;


#define READ_STATE_AUTO (&(Read_state){ .error = false, .path = { '\0' },\
                         .row = 0, .message = { '\0' } })


#define check_next(str, state, expect)                \
    if (true)                                         \
    {                                                 \
        (str) = read_const_char((str), ',', (state)); \
        if ((state)->error)                           \
        {                                             \
            Read_state_clear_error((state));          \
            (expect) = false;                         \
        }                                             \
    } else (void)0


/**
 * Initialises a Read state.
 *
 * \param state   The Read state -- must not be \c NULL.
 * \param path    The path of the current file -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Read_state* Read_state_init(Read_state* state, const char* path);


/**
 * Sets an error in the Read state.
 *
 * \param state     The Read state -- must not be \c NULL.
 * \param message   The error message format -- must not be \c NULL. This and
 *                  subsequent arguments follow the printf family conventions.
 */
void Read_state_set_error(Read_state* state, const char* message, ...);


/**
 * Clears the error in the Read state.
 *
 * \param state   The Read state -- must not be \c NULL.
 */
void Read_state_clear_error(Read_state* state);


/**
 * Reads a null value.
 *
 * \param str     The input string -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_null(char* str, Read_state* state);


/**
 * Reads a specified character.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The character to be matched.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_const_char(char* str, char result, Read_state* state);


/**
 * Reads a constant string.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The string to be matched -- must not be \c NULL.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_const_string(char* str, char* result, Read_state* state);


/**
 * Reads a Boolean value.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The address where the Boolean value will be stored, or
 *                 \c NULL for parsing without storing the value.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_bool(char* str, bool* result, Read_state* state);


/**
 * Reads a string.
 *
 * \param str       The input string -- must not be \c NULL.
 * \param result    The address where the string will be stored, or \c NULL
 *                  for parsing without storing the value.
 * \param max_len   The maximum number of characters to be written, including
 *                  the terminating '\0'.
 * \param state     The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_string(char* str, char* result, int max_len, Read_state* state);


/**
 * Reads an integer value.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The address where the integer value will be stored, or
 *                 \c NULL for parsing without storing the value.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_int(char* str, int64_t* result, Read_state* state);


/**
 * Reads a double-precision floating point value.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The address where the double value will be stored, or
 *                 \c NULL for parsing without storing the value.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_double(char* str, double* result, Read_state* state);


/**
 * Reads a tuning specification.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The address where the Real value will be stored (if found),
 *                 or \c NULL for parsing without storing the value.
 * \param cents    The address where the tuning in cents will be stored. If it
 *                 is not \c NULL and the tuning is not in cents, *cents will
 *                 be a NaN.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_tuning(char* str, Real* result, double* cents, Read_state* state);


/**
 * Reads a Reltime value.
 *
 * \param str      The input string -- must not be \c NULL.
 * \param result   The address where the Reltime value will be stored, or
 *                 \c NULL for parsing without storing the value.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* read_reltime(char* str, Reltime* result, Read_state* state);


#endif // K_FILE_BASE_H


