

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

#include <Channel.h>
#include <Channel_state.h>
#include <Event_common.h>
#include <Event_channel_set_dsp.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_dsp_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_DSP_EFFECTS_MAX - 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_channel_set_dsp,
                                   EVENT_CHANNEL_SET_DSP,
                                   int64_t, dsp);


Event_create_constructor(Event_channel_set_dsp,
                         EVENT_CHANNEL_SET_DSP,
                         set_dsp_desc,
                         event->dsp = 0);


bool Event_channel_set_dsp_process(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_dsp_desc, data, state);
    if (state->error)
    {
        return false;
    }
    ch_state->dsp = data[0].field.integral_type;
    return true;
}


