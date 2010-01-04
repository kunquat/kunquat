

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


