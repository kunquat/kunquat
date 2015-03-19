

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
#include <player/events/Event_common.h>
#include <player/events/Event_ins_decl.h>
#include <player/Proc_state.h>
#include <Value.h>


bool Event_ins_bypass_on_process(
        const Instrument* ins,
        const Instrument_params* ins_params,
        Ins_state* ins_state,
        Device_states* dstates,
        const Value* value)
{
    assert(ins != NULL);
    assert(ins_params != NULL);
    assert(ins_state != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    (void)ins_params;
    (void)value;

    ins_state->bypass = true;

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        const Processor* proc = Instrument_get_proc(ins, i);
        if (proc != NULL)
        {
            Proc_state* proc_state = (Proc_state*)Device_states_get_state(
                    dstates, Device_get_id((const Device*)proc));
            Processor_clear_history(proc, proc_state);
        }
    }

    return true;
}


bool Event_ins_bypass_off_process(
        const Instrument* ins,
        const Instrument_params* ins_params,
        Ins_state* ins_state,
        Device_states* dstates,
        const Value* value)
{
    assert(ins != NULL);
    assert(ins_params != NULL);
    assert(ins_state != NULL);
    assert(dstates != NULL);
    assert(value != NULL);
    (void)ins;
    (void)ins_params;
    (void)dstates;
    (void)value;

    ins_state->bypass = false;

    return true;
}


