

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/events/Event_au_decl.h>
#include <Value.h>


bool Event_au_set_sustain_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Device_states* dstates,
        const Value* value)
{
    assert(au != NULL);
    assert(au_params != NULL);
    assert(au_state != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_FLOAT);
    (void)au;
    (void)au_params;
    (void)dstates;

    au_state->sustain = value->value.float_type;

    return true;
}


