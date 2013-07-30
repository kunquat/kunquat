

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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

#include <Effect.h>
#include <Event_common.h>
#include <Event_effect_decl.h>
#include <Value.h>
#include <xassert.h>


bool Event_effect_bypass_on_process(
        Effect* eff,
        Effect_state* eff_state,
        Value* value)
{
    assert(eff != NULL);
    assert(eff_state != NULL);
    (void)value;

    eff_state->bypass = true;

    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = Effect_get_dsp(eff, i);
        if (dsp != NULL)
            DSP_clear_history(dsp);
    }

    return true;
}


bool Event_effect_bypass_off_process(
        Effect* eff,
        Effect_state* eff_state,
        Value* value)
{
    assert(eff != NULL);
    assert(eff_state != NULL);
    (void)value;

    eff_state->bypass = false;

    return true;
}


