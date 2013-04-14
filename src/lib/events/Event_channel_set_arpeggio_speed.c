

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
#include <float.h>

#include <Event.h>
#include <Event_common.h>
#include <Event_channel_set_arpeggio_speed.h>
#include <File_base.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_set_arpeggio_speed_process(Channel_state* ch_state,
                                              Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    ch_state->arpeggio_speed = value->value.float_type;
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       *ch_state->tempo,
                                       *ch_state->freq);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_length = unit_len / value->value.float_type;
    }
    return true;
}


