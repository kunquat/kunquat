

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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

#include <Event_common.h>
#include <Event_ins_set_sustain.h>
#include <Instrument_params.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


bool Event_ins_set_sustain_process(Instrument_params* ins_state, Value* value)
{
    assert(ins_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ins_state->sustain = value->value.float_type;
    return true;
}


