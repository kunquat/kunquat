

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <math.h>

#include <Event_common.h>
#include <Event_channel_vibrato_speed.h>
#include <math_common.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>


bool Event_channel_vibrato_speed_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ch_state->vibrato_speed = value->value.float_type;
    LFO_set_speed(&ch_state->vibrato, value->value.float_type);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        if (ch_state->fg[i]->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
        Voice_state* vs = ch_state->fg[i]->state;
        LFO_set_speed(&vs->vibrato, value->value.float_type);
        if (ch_state->vibrato_depth > 0)
        {
            LFO_set_depth(&vs->vibrato, ch_state->vibrato_depth);
        }
        LFO_turn_on(&vs->vibrato);
    }
    return true;
}


