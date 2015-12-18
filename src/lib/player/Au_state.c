

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Au_state.h>

#include <debug/assert.h>

#include <stdbool.h>
#include <stdlib.h>


void Au_state_reset(Au_state* au_state)
{
    assert(au_state != NULL);

    Device_state_reset(&au_state->parent);
    au_state->bypass = false;
    au_state->sustain = 0.0;

    return;
}


