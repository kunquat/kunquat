

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
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
#include <devices/Effect.h>
#include <player/DSP_state.h>
#include <player/events/Event_common.h>
#include <player/events/Event_effect_decl.h>
#include <Value.h>


bool Event_effect_bypass_on_process(
        const Effect* eff,
        Effect_state* eff_state,
        Device_states* dstates,
        const Value* value)
{
    assert(eff != NULL);
    assert(eff_state != NULL);
    assert(dstates != NULL);
    (void)eff;
    (void)value;

    eff_state->bypass = true;

    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        const DSP* dsp = Effect_get_dsp(eff, i);
        if (dsp != NULL)
        {
            DSP_state* dsp_state = (DSP_state*)Device_states_get_state(
                    dstates,
                    Device_get_id((const Device*)dsp));
            DSP_clear_history(dsp, dsp_state);
        }
    }

    return true;
}


bool Event_effect_bypass_off_process(
        const Effect* eff,
        Effect_state* eff_state,
        Device_states* dstates,
        const Value* value)
{
    assert(eff != NULL);
    assert(eff_state != NULL);
    assert(dstates != NULL);
    (void)eff;
    (void)dstates;
    (void)value;

    eff_state->bypass = false;

    return true;
}


