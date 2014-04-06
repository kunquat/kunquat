

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VALUE_H
#define K_VALUE_H


#include <stdbool.h>
#include <stdint.h>

#include <kunquat/limits.h>
#include <Pat_inst_ref.h>
#include <Real.h>
#include <Tstamp.h>


typedef enum
{
    VALUE_TYPE_NONE = -1,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_REAL,
    VALUE_TYPE_TSTAMP,
    VALUE_TYPE_STRING,
    VALUE_TYPE_PAT_INST_REF,
    VALUE_TYPE_COUNT,
} Value_type;


typedef struct Value
{
    Value_type type;
    union
    {
        bool bool_type;
        int64_t int_type;
        double float_type;
        Real Real_type;
        Tstamp Tstamp_type;
        char string_type[ENV_VAR_NAME_MAX];
        Pat_inst_ref Pat_inst_ref_type;
    } value;
} Value;


#define VALUE_AUTO (&(Value){ .type = VALUE_TYPE_NONE })


/**
 * Make a copy of a Value.
 *
 * \param dest   The destination Value -- must not be \c NULL.
 * \param src    The source Value -- must not be \c NULL or \a dest.
 *
 * \return   The parameter \a dest.
 */
Value* Value_copy(Value* restrict dest, const Value* restrict src);


/**
 * Convert a Value to another type.
 *
 * \param dest       The destination Value -- must not be \c NULL.
 * \param src        The source Value -- must not be \c NULL (but may be
 *                   \a dest).
 * \param new_type   The new Value type -- must be valid.
 *
 * \return   \c true if successful, or \c false if \a src cannot be converted
 *           to \a new_type.
 */
bool Value_convert(Value* dest, const Value* src, Value_type new_type);


/**
 * Serialise a Value.
 *
 * \param value   The Value -- must not be \c NULL.
 * \param len     Maximum amount of bytes to be written -- must be positive.
 * \param str     The output location -- must not be \c NULL.
 *
 * \return   The amount of bytes actually written to \a str, not including
 *           the terminating byte.
 */
int Value_serialise(Value* value, int len, char* str);


#endif // K_VALUE_H


