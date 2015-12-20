

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
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

#include <stdlib.h>


static const char* limit_names[] =
{
#define KQT_LIMIT_INT(name) #name,
#include <limits_foreign_inc.h>
    NULL
};


const char** kqt_get_int_limit_names(void)
{
    return limit_names;
}


static const struct
{
    const char* name;
    const long long value;
} limit_names_to_values[] =
{
#define KQT_LIMIT_INT(name) { #name, KQT_##name },
#include <limits_foreign_inc.h>
    { NULL, 0 }
};


long long kqt_get_int_limit(const char* limit_name)
{
    if (limit_name == NULL)
        return 0;

    for (int i = 0; limit_names_to_values[i].name != NULL; ++i)
    {
        if (string_eq(limit_name, limit_names_to_values[i].name))
            return limit_names_to_values[i].value;
    }

    return 0;
}


