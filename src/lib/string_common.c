

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <string_common.h>


bool string_has_prefix(const char* str, const char* prefix)
{
    if (prefix == NULL || *prefix == '\0')
    {
        return true;
    }
    if (str == NULL || *str == '\0')
    {
        return false;
    }
    return strncmp(str, prefix, strlen(prefix)) == 0;
}


bool string_has_suffix(const char* str, const char* suffix)
{
    if (suffix == NULL || *suffix == '\0')
    {
        return true;
    }
    if (str == NULL || *str == '\0')
    {
        return false;
    }
    if (strlen(suffix) > strlen(str))
    {
        return false;
    }
    const char* search = str + (strlen(str) - strlen(suffix));
    assert(strlen(search) == strlen(suffix));
    return strcmp(search, suffix) == 0;
}


