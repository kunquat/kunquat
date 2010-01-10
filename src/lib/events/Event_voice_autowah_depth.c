

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_autowah_depth.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc autowah_depth_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_autowah_depth,
                                   EVENT_VOICE_AUTOWAH_DEPTH,
                                   double, depth)


static void Event_voice_autowah_depth_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_autowah_depth,
                         EVENT_VOICE_AUTOWAH_DEPTH,
                         autowah_depth_desc,
                         event->depth = 0)


static void Event_voice_autowah_depth_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_AUTOWAH_DEPTH);
    assert(voice != NULL);
    Event_voice_autowah_depth* autowah_depth = (Event_voice_autowah_depth*)event;
    if (autowah_depth->depth > 0 && voice->state.generic.autowah_length > 0)
    {
        voice->state.generic.autowah = true;
    }
    voice->state.generic.autowah_depth_target = autowah_depth->depth / 8;
    voice->state.generic.autowah_delay_pos = 0;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->autowah_depth = voice->state.generic.autowah_depth_target;
    return;
}


