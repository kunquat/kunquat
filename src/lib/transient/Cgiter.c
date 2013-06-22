

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


#include <transient/Cgiter.h>
#include <xassert.h>


void Cgiter_init(Cgiter* cgiter)
{
    assert(cgiter != NULL);

    Tstamp_set(&cgiter->ref_pos, 0, 0);

    return;
}


