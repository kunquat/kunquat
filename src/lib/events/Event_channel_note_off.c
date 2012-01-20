

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
#include <stdio.h>

#include <Event_common.h>
#include <Event_channel_note_off.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc note_off_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_NOTE_OFF,
                         note_off);


bool Event_channel_note_off_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    (void)value;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (ch_state->fg[i] != NULL)
        {
            ch_state->fg[i] = Voice_pool_get_voice(ch_state->pool,
                                                   ch_state->fg[i],
                                                   ch_state->fg_id[i]);
            if (ch_state->fg[i] == NULL)
            {
                // The Voice has been given to another channel
                continue;
            }
            ch_state->fg[i]->state->note_on = false;
            ch_state->fg[i]->prio = VOICE_PRIO_BG;
            Voice_pool_fix_priority(ch_state->pool, ch_state->fg[i]);
            ch_state->fg[i] = NULL;
        }
    }
    ch_state->fg_count = 0;
    return true;
}


