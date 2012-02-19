

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

#include <Event_common.h>
#include <Event_channel_hit.h>
#include <Event_channel_note_off.h>
#include <note_setup.h>
#include <Reltime.h>
#include <Value.h>
#include <Voice.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_HIT,
                         hit);


bool Event_channel_hit_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(ch_state->freq != NULL);
    assert(ch_state->tempo != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT)
    {
        return false;
    }
    // move the old Voices to the background
    Event_channel_note_off_process(ch_state, NULL);
    ch_state->fg_count = 0;
    assert(ch_state->instrument >= 0);
    assert(ch_state->instrument < KQT_INSTRUMENTS_MAX);
    Instrument* ins = Ins_table_get(ch_state->insts,
                                    ch_state->instrument);
    if (ins == NULL)
    {
        return true;
    }
    double force_var = NAN;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (Instrument_get_gen(ins, i) == NULL)
        {
            continue;
        }
        reserve_voice(ch_state, ins, i);
        Voice* voice = ch_state->fg[i];
        Voice_state* vs = voice->state;
        vs->hit_index = value->value.int_type;
        set_instrument_properties(voice, vs, ch_state, &force_var);
    }
    return true;
}


