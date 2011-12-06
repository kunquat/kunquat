

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
#include <Event_channel_set_arpeggio_speed.h>
#include <File_base.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_arpeggio_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = DBL_MIN,
        .max.field.double_type = DBL_MAX
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SET_ARPEGGIO_SPEED,
                         set_arpeggio_speed);


bool Event_channel_set_arpeggio_speed_process(Channel_state* ch_state,
                                              char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_arpeggio_speed_desc, data, state);
    if (state->error)
    {
        return false;
    }
    ch_state->arpeggio_speed = data[0].field.double_type;
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       *ch_state->tempo,
                                       *ch_state->freq);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->arpeggio_length = unit_len / data[0].field.double_type;
    }
    return true;
}


