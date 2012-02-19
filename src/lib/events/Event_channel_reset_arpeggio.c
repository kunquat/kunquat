

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
#include <math.h>

#include <Event.h>
#include <Event_common.h>
#include <Event_channel_reset_arpeggio.h>
#include <File_base.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


bool Event_channel_reset_arpeggio_process(Channel_state* ch_state,
                                          Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    ch_state->arpeggio_ref = NAN;
    ch_state->arpeggio_edit_pos = 1;
    ch_state->arpeggio_tones[0] = ch_state->arpeggio_tones[1] = NAN;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_tones[0] = vs->arpeggio_tones[1] = NAN;
        vs->arpeggio_note = 0;
    }
    return true;
}


