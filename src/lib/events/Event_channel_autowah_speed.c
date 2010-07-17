

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
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_channel_autowah_speed.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc autowah_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = 0,
        .max.field.double_type = INFINITY
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_channel_autowah_speed,
                                   EVENT_CHANNEL_AUTOWAH_SPEED,
                                   double, speed);


Event_create_constructor(Event_channel_autowah_speed,
                         EVENT_CHANNEL_AUTOWAH_SPEED,
                         autowah_speed_desc,
                         event->speed = 0);


bool Event_channel_autowah_speed_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, autowah_speed_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       *ch_state->tempo,
                                       *ch_state->freq);
    ch_state->autowah_length = unit_len / data[0].field.double_type;
    ch_state->autowah_update = (2 * PI) / ch_state->autowah_length;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        if (data[0].field.double_type > 0 && vs->autowah_depth_target > 0)
        {
            vs->autowah = true;
        }
        vs->autowah_length = ch_state->autowah_length;
        vs->autowah_update = ch_state->autowah_update;
        if (!vs->autowah)
        {
            vs->autowah_delay_pos = 0;
        }
    }
    return true;
}


