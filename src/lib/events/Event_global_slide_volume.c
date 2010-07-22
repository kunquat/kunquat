

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
#include <limits.h>
#include <math.h>

#include <Event_common.h>
#include <Event_global_slide_volume.h>
#include <Reltime.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc slide_volume_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = 0
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_global,
                         EVENT_GLOBAL_SLIDE_VOLUME,
                         slide_volume);


bool Event_global_slide_volume_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_volume_desc, data, state);
    if (state->error)
    {
        return false;
    }
    double target = exp2(data[0].field.double_type / 6);
    Slider_set_mix_rate(&global_state->volume_slider, global_state->freq);
    Slider_set_tempo(&global_state->volume_slider, global_state->tempo);
    if (Slider_in_progress(&global_state->volume_slider))
    {
        Slider_change_target(&global_state->volume_slider, target);
    }
    else
    {
        Slider_start(&global_state->volume_slider,
                     target, global_state->volume);
    }
#if 0
    global_state->volume_slide_target = exp2(data[0].field.double_type / 6);
    global_state->volume_slide_frames =
            Reltime_toframes(&global_state->volume_slide_length,
                             global_state->tempo,
                             global_state->freq);
    double volume_dB = log2(global_state->volume) * 6;
    double dB_step = (data[0].field.double_type - volume_dB) /
                     global_state->volume_slide_frames;
    global_state->volume_slide_update = exp2(dB_step / 6);
    if (dB_step > 0)
    {
        global_state->volume_slide = 1;
    }
    else if (dB_step < 0)
    {
        global_state->volume_slide = -1;
    }
    else
    {
        global_state->volume = global_state->volume_slide_target;
    }
#endif
    return true;
}


