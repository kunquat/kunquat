

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_STREADER_H
#define KQT_STREADER_H


#include <decl.h>
#include <Error.h>
#include <mathnum/Tstamp.h>
#include <Pat_inst_ref.h>
#include <Value.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define STREADER_DICT_KEY_LENGTH_MAX 128


/**
 * A string reader that supports char arrays that are not null terminated.
 */
struct Streader
{
    int64_t pos;
    int64_t len;
    int line;
    const char* str;
    Error error;
};

#define STREADER_AUTO (&(Streader){ .pos = 0, .len = 0, .str = "" })


/**
 * A helper structure used to catch inadvertent misuse of formatted string reading.
 */
typedef struct Streader_readf_str_info
{
    int32_t guard;
    int64_t max_bytes;
    char* dest;
} Streader_readf_str_info;

static const int32_t Streader_readf_str_guard = (int32_t)0x7fffff7f;

#define READF_STR(mb, d) ((Streader_readf_str_info){ \
        .guard = Streader_readf_str_guard, .max_bytes = (mb), .dest = (d) })


/**
 * Initialise a Streader.
 *
 * \param sr    The Streader -- must not be \c NULL.
 * \param str   The input data -- must not be \c NULL unless the data
 *              length is \c 0.
 * \param len   The length of the data -- must be >= \c 0.
 *
 * \return   The parameter \a sr.
 */
Streader* Streader_init(Streader* sr, const char* str, int64_t len);


/**
 * Find out if the Streader error is set.
 *
 * \param sr   The Streader -- must not be \c NULL.
 *
 * \return   \c true if the error is set, otherwise \c false.
 */
bool Streader_is_error_set(const Streader* sr);


/**
 * Get the Streader error description.
 *
 * \param sr   The Streader -- must not be \c NULL.
 *
 * \return   The error description in JSON format, or an empty string.
 */
const char* Streader_get_error_desc(const Streader* sr);


/**
 * Set a parse error in the Streader.
 *
 * \param sr       The Streader -- must not be \c NULL.
 * \param format   The error message format -- must not be \c NULL. This and
 *                 subsequent arguments follow the printf family conventions.
 */
void Streader_set_error(Streader* sr, const char* format, ...);


/**
 * Set a memory error in the Streader.
 *
 * \param sr       The Streader -- must not be \c NULL.
 * \param format   The error message format -- must not be \c NULL. This and
 *                 subsequent arguments follow the printf family conventions.
 */
void Streader_set_memory_error(Streader* sr, const char* format, ...);


/**
 * Clear the error in the Streader.
 *
 * \param sr   The Streader -- must not be \c NULL.
 */
void Streader_clear_error(Streader* sr);


/**
 * Return the suffix of the data starting from current reading position.
 *
 * \param sr   The Streader -- must not be \c NULL.
 *
 * \return   The address of the suffix, or \c NULL if there is no data left.
 */
const char* Streader_get_remaining_data(const Streader* sr);


/**
 * Skip whitespace.
 *
 * This function is not needed for reading JSON data.
 *
 * \param sr   The Streader -- must not be \c NULL.
 *
 * \return   \c true if \a sr did not have error set, otherwise \c false.
 */
bool Streader_skip_whitespace(Streader* sr);


/**
 * Find out if the Streader contains JSON data.
 *
 * \param sr   The Streader -- must not be \c NULL and must not have error set.
 *
 * \return   \c true if \a sr contains JSON data, otherwise \c false.
 */
bool Streader_has_data(Streader* sr);


/**
 * Match a specified character.
 *
 * \param sr   The Streader -- must not be \c NULL.
 * \param ch   The character to be matched.
 *
 * \return   \c true if \a ch was successfully matched, otherwise \c false.
 */
bool Streader_match_char(Streader* sr, char ch);


/**
 * Try to match a specified character.
 *
 * If matching fails, the reading position before the call is restored and the
 * error status is cleared.
 *
 * \param sr   The Streader -- must not be \c NULL and must not have error set.
 * \param ch   The character to be matched.
 *
 * \return   \c true if \a ch was successfully matched, otherwise \c false.
 */
bool Streader_try_match_char(Streader* sr, char ch);


/**
 * Match a specified string.
 *
 * \param sr    The Streader -- must not be \c NULL.
 * \param str   The string to be matched -- must not be \c NULL.
 *
 * \return   \c true if \a str was successfully matched, otherwise \c false.
 */
bool Streader_match_string(Streader* sr, const char* str);


/**
 * Read a null value.
 *
 * \param sr   The Streader -- must not be \c NULL.
 *
 * \return   \c true if a null was successfully read, otherwise \c false.
 */
bool Streader_read_null(Streader* sr);


/**
 * Read a Boolean value.
 *
 * \param sr     The Streader -- must not be \c NULL.
 * \param dest   The destination address of the Boolean value, or
 *               \c NULL for parsing without storing the value.
 *
 * \return   \c true if a bool was successfully read, otherwise \c false.
 */
bool Streader_read_bool(Streader* sr, bool* dest);


/**
 * Read an integer value.
 *
 * \param sr     The Streader -- must not be \c NULL.
 * \param dest   The destination address of the integer value, or
 *               \c NULL for parsing without storing the value.
 *
 * \return   \c true if an integer was successfully read, otherwise \c false.
 */
bool Streader_read_int(Streader* sr, int64_t* dest);


/**
 * Read a float value.
 *
 * NOTE: This function does not give portable results.
 *
 * \param sr     The Streader -- must not be \c NULL.
 * \param dest   The destination address of the float value, or
 *               \c NULL for parsing without storing the value.
 *
 * \return   \c true if a float was successfully read, otherwise \c false.
 */
bool Streader_read_float(Streader* sr, double* dest);


/**
 * Read a string value.
 *
 * \param sr          The Streader -- must not be \c NULL.
 * \param max_bytes   The maximum number of bytes to be written, including
 *                    the terminating '\0' -- must be >= \c 0.
 * \param dest        The destination address of the string, or
 *                    \c NULL for parsing without storing the value.
 *
 * \return   \c true if a float was successfully read, otherwise \c false.
 */
bool Streader_read_string(Streader* sr, int64_t max_bytes, char* dest);


/**
 * Read a Tstamp value.
 *
 * \param sr     The Streader -- must not be \c NULL.
 * \param dest   The destination address of the Tstamp value, or
 *               \c NULL for parsing without storing the value.
 *
 * \return   \c true if a Tstamp was successfully read, otherwise \c false.
 */
bool Streader_read_tstamp(Streader* sr, Tstamp* dest);


/**
 * Read any finite realtime value (boolean, integer, float or Tstamp).
 *
 * \param sr     The Streader -- must not be \c NULL.
 * \param dest   The destination address of the Value container, or
 *               \c NULL for parsing without storing the value.
 *
 * \return   \c true if a finite realtime value was successfully read,
 *           otherwise \c false.
 */
bool Streader_read_finite_rt(Streader* sr, Value* dest);


/**
 * Read a Pattern instance reference.
 *
 * \param sr     The Streader -- must not be \c NULL.
 * \param dest   The destination address of the Pattern instance reference, or
 *               \c NULL for parsing without storing the value.
 *
 * \return   \c true if a Pattern instance reference was successfully read,
 *           otherwise \c false.
 */
bool Streader_read_piref(Streader* sr, Pat_inst_ref* dest);


/**
 * Callback function type for handling a list of values.
 *
 * \param sr         The Streader for the input data -- must not be \c NULL.
 * \param index      The index of the item to be read -- must be >= \c 0.
 * \param userdata   Callback userdata.
 *
 * \return   \c true if successful, otherwise \c false. The function must
 *           set the Streader error properly in case of error.
 */
typedef bool List_item_reader(Streader* sr, int32_t index, void* userdata);


/**
 * Read a list of values.
 *
 * \param sr         The Streader -- must not be \c NULL.
 * \param ir         A callback function for reading a list item, or \c NULL
 *                   if expecting an empty list.
 * \param userdata   Userdata passed to \a ir.
 *
 * \return   \c true if a list was successfully read, otherwise \c false.
 */
bool Streader_read_list(Streader* sr, List_item_reader ir, void* userdata);


/**
 * Callback function type for handling a dictionary.
 *
 * \param sr         The Streader for the input data -- must not be \c NULL.
 * \param key        The key of the item to be read -- must not be \c NULL.
 * \param userdata   Callback userdata.
 *
 * \return   \c true if successful, otherwise \c false. The function must
 *           set the Streader error properly in case of error.
 */
typedef bool Dict_item_reader(Streader* sr, const char* key, void* userdata);


/**
 * Read a dictionary.
 *
 * This function supports dictionary keys up to \c STREADER_DICT_KEY_LENGTH_MAX
 * characters in length.
 *
 * \param sr         The Streader -- must not be \c NULL.
 * \param ir         A callback function for reading a dictionary value,
 *                   or \c NULL if expecting an empty dictionary.
 * \param userdata   Userdata passed to \a ir.
 *
 * \return   \c true if a dictionary was successfully read, otherwise \c false.
 */
bool Streader_read_dict(Streader* sr, Dict_item_reader ir, void* userdata);


/**
 * Read data according to a given format string.
 *
 * This function uses a similar but not equivalent syntax to that used by
 * the scanf family of functions. The function supports conversion characters
 * that specify JSON data to be read. Conversion characters are prefixed by a
 * '%' character. Supported conversions are:
 *
 *  - n -- Read a null value.
 *  - b -- Read a boolean value and store it into a bool.
 *  - i -- Read an integer value and store it into an int64_t.
 *  - f -- Read a decimal number and store it into a double.
 *  - s -- Read a string of given maximum length (including null byte) and
 *         storage location passed with READF_STR().
 *  - t -- Read a timestamp and store it into a Tstamp.
 *  - p -- Read a Pattern instance reference and store it into a Pat_inst_ref.
 *  - l -- Read a list of values and call a given List_item_reader for each
 *         item with given userdata.
 *  - d -- Read a dictionary of values and call a given Dict_item_reader for
 *         each value of a key-value mapping with given userdata.
 *  - % -- Read '%'.
 *
 * Literal characters (not prefixed by a '%' character) must be matched
 * exactly, but whitespace between tokens in the JSON string is allowed.
 *
 * \param sr       The Streader -- must not be \c NULL.
 * \param format   The format string -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Streader_readf(Streader* sr, const char* format, ...);


#endif // KQT_STREADER_H


