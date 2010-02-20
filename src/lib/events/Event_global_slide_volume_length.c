

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
#include <limits.h>
#include <math.h>

#include <Event_common.h>
#include <Event_global_slide_volume_length.h>
#include <Reltime.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_volume_length_desc[] =
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


Event_create_set_reltime_and_get(Event_global_slide_volume_length,
                                 EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                                 length);


Event_create_constructor(Event_global_slide_volume_length,
                         EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                         slide_volume_length_desc,
                         Reltime_set(&event->length, 0, 0));


bool Event_global_slide_volume_length_handle(Playdata* global_state,
                                             char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_volume_length_desc, data, state);
    if (state->error)
    {
        return false;
    }
    if (global_state->volume_slide != 0)
    {
        global_state->volume_slide_frames =
                Reltime_toframes(&data[0].field.Reltime_type,
                                 global_state->tempo,
                                 global_state->freq);
        double volume_dB = log2(global_state->volume) * 6;
        double target_dB = log2(global_state->volume_slide_target) * 6;
        double dB_step = (target_dB - volume_dB) /
                         global_state->volume_slide_frames;
        global_state->volume_slide_update = exp2(dB_step / 6);
    }
    Reltime_copy(&global_state->volume_slide_length,
                 &data[0].field.Reltime_type);
    return true;
}


