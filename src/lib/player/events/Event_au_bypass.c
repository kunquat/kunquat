

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2017
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
#include <init/devices/Processor.h>
#include <player/devices/Proc_state.h>
#include <player/events/Event_common.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_au_bypass_on_process(
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

    au_state->bypass = true;

    return true;
}


bool Event_au_bypass_off_process(
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

    if (au_state->bypass)
    {
        for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
        {
            const Processor* proc = Audio_unit_get_proc(au, i);
            if ((proc != NULL) && Device_is_existent((const Device*)proc))
            {
                Proc_state* proc_state = (Proc_state*)Device_states_get_state(
                        dstates, Device_get_id((const Device*)proc));
                Proc_state_clear_history(proc_state);
            }
        }
    }

    au_state->bypass = false;

    return true;
}


