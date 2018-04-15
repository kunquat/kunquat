

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <kunquat/limits_foreign.h>

#include <kunquat/limits.h>
#include <string/common.h>
#include <Value.h>

#include <stdlib.h>


static const char* int_limit_names[] =
{
#define KQT_LIMIT_INT(name) #name,
#define KQT_LIMIT_STRING(name)
#include <limits_foreign_inc.h>
    NULL
};


static const char* string_limit_names[] =
{
#define KQT_LIMIT_INT(name)
#define KQT_LIMIT_STRING(name) #name,
#include <limits_foreign_inc.h>
    NULL
};


const char** kqt_get_int_limit_names(void)
{
    return int_limit_names;
}


const char** kqt_get_string_limit_names(void)
{
    return string_limit_names;
}


static const struct
{
    const char* name;
    Value_type type;
    union
    {
        const long long int_type;
        const char* string_type;
    } value;
} limit_names_to_values[] =
{
#define KQT_LIMIT_INT(name) { #name, VALUE_TYPE_INT, { .int_type = KQT_##name } },
#define KQT_LIMIT_STRING(name) \
    { #name, VALUE_TYPE_STRING, { .string_type = KQT_##name } },
#include <limits_foreign_inc.h>
    { NULL, VALUE_TYPE_NONE, { .int_type = 0 } }
};


long long kqt_get_int_limit(const char* limit_name)
{
    if (limit_name == NULL)
        return 0;

    for (int i = 0; limit_names_to_values[i].name != NULL; ++i)
    {
        if (string_eq(limit_name, limit_names_to_values[i].name) &&
                (limit_names_to_values[i].type == VALUE_TYPE_INT))
            return limit_names_to_values[i].value.int_type;
    }

    return 0;
}


const char* kqt_get_string_limit(const char* limit_name)
{
    if (limit_name == NULL)
        return NULL;

    for (int i = 0; limit_names_to_values[i].name != NULL; ++i)
    {
        if (string_eq(limit_name, limit_names_to_values[i].name) &&
                (limit_names_to_values[i].type == VALUE_TYPE_STRING))
            return limit_names_to_values[i].value.string_type;
    }

    return NULL;
}


