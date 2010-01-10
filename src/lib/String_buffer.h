

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef K_STRING_BUFFER_H
#define K_STRING_BUFFER_H


#include <stdbool.h>
#include <stdint.h>


/**
 * String buffer is a dynamically expanding buffer.
 */
typedef struct String_buffer String_buffer;


/**
 * Creates a new String buffer.
 *
 * \return   The new String buffer if successful, or \c NULL if memory
 *           allocation failed.
 */
String_buffer* new_String_buffer(void);


/**
 * Tells whether an error has occurred in the String buffer.
 *
 * \param sb   The String buffer -- must not be \c NULL.
 *
 * \return   \c true if an error has occurred, otherwise \c false. \c true
 *           always indicates a memory allocation error.
 */
bool String_buffer_error(String_buffer* sb);


/**
 * Tells the length of the String buffer.
 *
 * \param sb   The String buffer -- must not be \c NULL.
 *
 * \return   The length of the buffer.
 */
long String_buffer_get_length(String_buffer* sb);


/**
 * Appends a character into the String buffer.
 *
 * If an error has previously occurred while processing the given String
 * buffer, this function will do nothing. This is to reduce the need of
 * checking each append for errors.
 *
 * \param sb   The String buffer -- must not be \c NULL.
 * \param ch   The character to be inserted.
 *
 * \return   \c true if successful. Otherwise \c false is returned and the
 *           internal error flag of \a sb is set.
 */
bool String_buffer_append_ch(String_buffer* sb, char ch);


/**
 * Appends a string into the String buffer.
 *
 * If an error has previously occurred while processing the given String
 * buffer, this function will do nothing. This is to reduce the need of
 * checking each append for errors.
 *
 * \param sb       The String buffer -- must not be \c NULL.
 * \param string   The string to be inserted -- must not be \c NULL.
 *
 * \return   \c true if successful. Otherwise \c false is returned and the
 *           internal error flag of \a sb is set.
 */
bool String_buffer_append_string(String_buffer* sb, const char* str);


/**
 * Appends an integer into the String buffer.
 *
 * If an error has previously occurred while processing the given String
 * buffer, this function will do nothing. This is to reduce the need of
 * checking each append for errors.
 *
 * \param sb    The String buffer -- must not be \c NULL.
 * \param num   The number to be inserted.
 *
 * \return   \c true if successful. Otherwise \c false is returned and the
 *           internal error flag of \a sb is set.
 */
bool String_buffer_append_int(String_buffer* sb, int32_t num);


/**
 * Appends a floating point number into the String buffer.
 *
 * If an error has previously occurred while processing the given String
 * buffer, this function will do nothing. This is to reduce the need of
 * checking each append for errors.
 *
 * \param sb    The String buffer -- must not be \c NULL.
 * \param num   The number to be inserted.
 *
 * \return   \c true if successful. Otherwise \c false is returned and the
 *           internal error flag of \a sb is set.
 */
bool String_buffer_append_float(String_buffer* sb, double num);


/**
 * Gets the string inside the String buffer.
 *
 * \param sb   The String buffer -- must not be \c NULL.
 *
 * \return   The string. This is always a valid string, but it becomes invalid
 *           if String_buffer_append is called for \a sb afterwards.
 */
const char* String_buffer_get_string(String_buffer* sb);


/**
 * Destroys an existing String buffer.
 *
 * Note: This function returns the internal buffer instead of destroying it.
 * If you want to destroy the buffer completely, call
 * xfree(del_String_buffer(sb)).
 *
 * \param sb   The String buffer -- must not be \c NULL.
 *
 * \return   The string stored inside the String buffer. This is always a
 *           valid string.
 */
char* del_String_buffer(String_buffer* sb);


#endif // K_STRING_BUFFER_H


