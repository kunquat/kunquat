

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_au_decl.h>

#include <debug/assert.h>
#include <player/devices/Au_state.h>
#include <player/events/Event_common.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_au_set_sustain_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Value* value)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    au_state->sustain = value->value.float_type;

    return true;
}


