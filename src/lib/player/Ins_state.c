

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


#include <debug/assert.h>
#include <player/Ins_state.h>


void Ins_state_reset(Ins_state* ins_state)
{
    assert(ins_state != NULL);

    Device_state_reset(&ins_state->parent);
    ins_state->bypass = false;
    ins_state->sustain = 0.0;

    return;
}


