

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdio.h>

#include <manifest.h>
#include <string_common.h>
#include <xassert.h>


bool read_default_manifest(char* str, Read_state* state)
{
    assert(state != NULL);

    if (state->error)
        return false;

    if (string_eq(str, ""))
        return false;

    str = read_const_char(str, '{', state);
    str = read_const_char(str, '}', state);

    return !state->error;
}


