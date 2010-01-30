

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

#include <Event_common.h>
#include <Event_voice_tremolo_depth.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc tremolo_depth_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = 0,
        .max.field.double_type = 24
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_tremolo_depth,
                                   EVENT_VOICE_TREMOLO_DEPTH,
                                   double, depth)


static void Event_voice_tremolo_depth_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_tremolo_depth,
                         EVENT_VOICE_TREMOLO_DEPTH,
                         tremolo_depth_desc,
                         event->depth = 0)


static void Event_voice_tremolo_depth_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO_DEPTH);
    assert(voice != NULL);
    Event_voice_tremolo_depth* tremolo_depth = (Event_voice_tremolo_depth*)event;
    if (tremolo_depth->depth > 0 && voice->state.generic.tremolo_length > 0)
    {
        voice->state.generic.tremolo = true;
    }
    voice->state.generic.tremolo_depth_target = tremolo_depth->depth;
    voice->state.generic.tremolo_delay_pos = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->tremolo_depth = voice->state.generic.tremolo_depth_target;
    return;
}


