

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <debug/assert.h>
#include <player/Effect_state.h>


void Effect_state_reset(Effect_state* eff_state)
{
    assert(eff_state != NULL);

    Device_state_reset(&eff_state->parent);
    eff_state->bypass = false;

    return;
}


