

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
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

#include <Env_var.h>
#include <Real.h>
#include <Reltime.h>


typedef enum
{
    VALUE_TYPE_NONE = 0,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_REAL,
    VALUE_TYPE_TIMESTAMP,
    VALUE_TYPE_STRING,
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
        Reltime Timestamp_type;
        char string_type[ENV_VAR_NAME_MAX];
    } value;
} Value;


#define VALUE_AUTO (&(Value){ .type = VALUE_TYPE_NONE })


#endif // K_VALUE_H


