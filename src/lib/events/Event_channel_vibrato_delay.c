

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <Event_common.h>
#include <Event_channel_vibrato_delay.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc vibrato_delay_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .min.field.Reltime_type = { 0, 0 },
        .max.field.Reltime_type = { INT64_MAX, KQT_RELTIME_BEAT - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_channel_vibrato_delay,
                                 EVENT_CHANNEL_VIBRATO_DELAY,
                                 delay);


Event_create_constructor(Event_channel_vibrato_delay,
                         EVENT_CHANNEL_VIBRATO_DELAY,
                         vibrato_delay_desc,
                         Reltime_set(&event->delay, 0, KQT_RELTIME_BEAT / 4));


bool Event_channel_vibrato_delay_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, vibrato_delay_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double delay_frames = Reltime_toframes(&data[0].field.Reltime_type,
                                           *ch_state->tempo,
                                           *ch_state->freq);
    ch_state->vibrato_delay_update = 1 / delay_frames;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        vs->vibrato_delay_pos = 0;
        vs->vibrato_delay_update = ch_state->vibrato_delay_update;
        if (vs->vibrato_delay_update == 0)
        {
            vs->vibrato_delay_pos = 1;
        }
    }
    return true;
}


