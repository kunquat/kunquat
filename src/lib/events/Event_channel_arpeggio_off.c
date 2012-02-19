

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
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
#include <string.h>
#include <math.h>

#include <Event_common.h>
#include <Event_channel_arpeggio_off.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_ARPEGGIO_OFF,
                         arpeggio_off);


bool Event_channel_arpeggio_off_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        ch_state->fg[i]->state->arpeggio = false;
    }
    return true;
}


