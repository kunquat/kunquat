

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


#include <string/var_name.h>

#include <debug/assert.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static bool is_valid_var_name_with_length(const char* str, size_t length)
{
    rassert(str != NULL);

    return (0 < length) && (length <= KQT_VAR_NAME_MAX) &&
        (strspn(str, KQT_VAR_CHARS) == length) &&
        (strchr(KQT_VAR_INIT_CHARS, str[0]) != NULL);
}


bool is_valid_var_name(const char* str)
{
    rassert(str != NULL);

    const size_t length = strlen(str);
    return is_valid_var_name_with_length(str, length);
}


bool is_valid_var_path(const char* str)
{
    rassert(str != NULL);

    const char* part = str;
    do
    {
        const size_t part_length = strcspn(part, "/");
        if (!is_valid_var_name_with_length(part, part_length))
            return false;

        part += part_length;
        if (*part == '/')
            ++part;
        else
            rassert(*part == '\0');
    } while (*part != '\0');

    return true;
}


