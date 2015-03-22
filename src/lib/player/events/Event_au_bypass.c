

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
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
#include <devices/Processor.h>
#include <player/events/Event_au_decl.h>
#include <player/events/Event_common.h>
#include <player/Proc_state.h>
#include <Value.h>


bool Event_au_bypass_on_process(
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

    au_state->bypass = true;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Audio_unit_get_proc(au, i);
        if (proc != NULL)
        {
            Proc_state* proc_state = (Proc_state*)Device_states_get_state(
                    dstates, Device_get_id((const Device*)proc));
            Processor_clear_history(proc, proc_state);
        }
    }

    return true;
}


bool Event_au_bypass_off_process(
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

    au_state->bypass = false;

    return true;
}


