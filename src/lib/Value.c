

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


#include <stdlib.h>
#include <string.h>

#include <Value.h>
#include <xassert.h>


Value* Value_copy(Value* restrict dest, const Value* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(dest != src);
    memcpy(dest, src, sizeof(Value));
    return dest;
}


