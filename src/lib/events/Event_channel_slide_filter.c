

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
#include <math.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_channel_slide_filter.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_filter_desc[] =
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


Event_create_set_primitive_and_get(Event_channel_slide_filter,
                                   EVENT_CHANNEL_SLIDE_FILTER,
                                   double, target_cutoff);


Event_create_constructor(Event_channel_slide_filter,
                         EVENT_CHANNEL_SLIDE_FILTER,
                         slide_filter_desc,
                         event->target_cutoff = 90);


bool Event_channel_slide_filter_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_filter_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double target_cutoff = data[0].field.double_type;
    double target_cutoff_exp = NAN;
    if (target_cutoff > 86)
    {
        target_cutoff_exp = INFINITY;
    }
    else
    {
        target_cutoff_exp = exp2((target_cutoff + 86) / 12);
    }
    const double inf_limit = exp2((86.0 + 86) / 12);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        vs->filter_slide_target = target_cutoff_exp;
        vs->filter_slide_frames =
                Reltime_toframes(&vs->filter_slide_length,
                                 *ch_state->tempo,
                                 *ch_state->freq);
        if (vs->filter > inf_limit)
        {
            vs->filter = inf_limit;
        }
        double diff_log = target_cutoff - (log2(vs->filter) * 12 - 86);
        double slide_step = diff_log / vs->filter_slide_frames;
        vs->filter_slide_update = exp2(slide_step / 12);
        if (slide_step > 0)
        {
            vs->filter_slide = 1;
        }
        else if (slide_step < 0)
        {
            vs->filter_slide = -1;
        }
        else
        {
            vs->filter_slide = 0;
            vs->filter = vs->filter_slide_target;
        }
    }
    return true;
}


