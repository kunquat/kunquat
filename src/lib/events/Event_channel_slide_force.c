

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
#include <limits.h>

#include <Event_common.h>
#include <Event_channel_slide_force.h>
#include <Reltime.h>
#include <Voice.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc slide_force_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = 18
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_channel_slide_force,
                                   EVENT_CHANNEL_SLIDE_FORCE,
                                   double, target_force_dB);


Event_create_constructor(Event_channel_slide_force,
                         EVENT_CHANNEL_SLIDE_FORCE,
                         slide_force_desc,
                         event->target_force_dB = 0);


bool Event_channel_slide_force_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_force_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double slide_target = exp2(data[0].field.double_type / 6);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = ch_state->fg[i]->state;
        vs->force_slide_target = slide_target;
        vs->force_slide_frames = Reltime_toframes(&vs->force_slide_length,
                                                  *ch_state->tempo,
                                                  *ch_state->freq);
        double force_dB = log2(vs->force) * 6;
        double dB_step = (data[0].field.double_type - force_dB) /
                         vs->force_slide_frames;
        vs->force_slide_update = exp2(dB_step / 6);
        if (dB_step > 0)
        {
            vs->force_slide = 1;
        }
        else if (dB_step < 0)
        {
            vs->force_slide = -1;
        }
        else
        {
            vs->force = vs->force_slide_target;
            vs->force_slide = 0;
        }
    }
    return true;
}


