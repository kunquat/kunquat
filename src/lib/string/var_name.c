

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


#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <string/var_name.h>


bool is_valid_var_name(const char* str)
{
    assert(str != NULL);

    const size_t length = strlen(str);

    return (0 < length) && (length < KQT_VAR_NAME_MAX) &&
        (strspn(str, KQT_VAR_CHARS) == length) &&
        (strchr(KQT_VAR_INIT_CHARS, str[0]) != NULL);
}


