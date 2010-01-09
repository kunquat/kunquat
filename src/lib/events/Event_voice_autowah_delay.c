

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_autowah_delay.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc autowah_delay_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_voice_autowah_delay,
                                 EVENT_VOICE_AUTOWAH_DELAY,
                                 delay)


static void Event_voice_autowah_delay_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_autowah_delay,
                         EVENT_VOICE_AUTOWAH_DELAY,
                         autowah_delay_desc,
                         Reltime_set(&event->delay, 0, KQT_RELTIME_BEAT / 4))


static void Event_voice_autowah_delay_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_AUTOWAH_DELAY);
    assert(voice != NULL);
    Event_voice_autowah_delay* autowah_delay = (Event_voice_autowah_delay*)event;
    double delay_frames = Reltime_toframes(&autowah_delay->delay,
                                           voice->state.generic.tempo,
                                           voice->state.generic.freq);
    voice->state.generic.autowah_delay_pos = 0;
    voice->state.generic.autowah_delay_update = 1 / delay_frames;
    if (voice->state.generic.autowah_delay_update == 0)
    {
        voice->state.generic.autowah_delay_pos = 1;
    }
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->autowah_delay_update = voice->state.generic.autowah_delay_update;
    return;
}


