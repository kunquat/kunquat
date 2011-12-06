

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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
#include <Event_channel_set_arpeggio_note.h>
#include <File_base.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_arpeggio_note_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -DBL_MAX,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SET_ARPEGGIO_NOTE,
                         set_arpeggio_note);


bool Event_channel_set_arpeggio_note_process(Channel_state* ch_state,
                                             char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_arpeggio_note_desc, data, state);
    if (state->error)
    {
        return false;
    }
    if (isnan(ch_state->arpeggio_tones[ch_state->arpeggio_edit_pos]) &&
            ch_state->arpeggio_edit_pos < KQT_ARPEGGIO_NOTES_MAX - 1)
    {
        ch_state->arpeggio_tones[ch_state->arpeggio_edit_pos + 1] = NAN;
        for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
        {
            Event_check_voice(ch_state, i);
            Voice_state* vs = ch_state->fg[i]->state;
            vs->arpeggio_tones[ch_state->arpeggio_edit_pos + 1] = NAN;
        }
    }
    ch_state->arpeggio_tones[ch_state->arpeggio_edit_pos] =
            data[0].field.double_type;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_tones[ch_state->arpeggio_edit_pos] =
                data[0].field.double_type;
    }
    if (ch_state->arpeggio_edit_pos < KQT_ARPEGGIO_NOTES_MAX - 1)
    {
        ++ch_state->arpeggio_edit_pos;
    }
    return true;
}


