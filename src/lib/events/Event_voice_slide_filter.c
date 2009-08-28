

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_voice_slide_filter.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_filter_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_voice_slide_filter,
                                   EVENT_VOICE_SLIDE_FILTER,
                                   double, target_cutoff)


static void Event_voice_slide_filter_process(Event_voice* event, Voice* voice);


Event_create_constructor(Event_voice_slide_filter,
                         EVENT_VOICE_SLIDE_FILTER,
                         slide_filter_desc,
                         event->target_cutoff = 90)


static void Event_voice_slide_filter_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_FILTER);
    assert(voice != NULL);
    Event_voice_slide_filter* slide_filter = (Event_voice_slide_filter*)event;
    double target_cutoff = slide_filter->target_cutoff;
    if (target_cutoff > 86)
    {
        target_cutoff = 86;
        voice->state.generic.filter_slide_target = INFINITY;
    }
    else
    {
        voice->state.generic.filter_slide_target = exp2((slide_filter->target_cutoff + 86) / 12);
    }
    voice->state.generic.filter_slide_frames =
            Reltime_toframes(&voice->state.generic.filter_slide_length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    double inf_limit = exp2((86.0 + 86) / 12);
    if (voice->state.generic.filter > inf_limit)
    {
        voice->state.generic.filter = inf_limit;
    }
    double diff_log = target_cutoff -
            (log2(voice->state.generic.filter) * 12 - 86);
    double slide_step = diff_log / voice->state.generic.filter_slide_frames;
    voice->state.generic.filter_slide_update = exp2(slide_step / 12);
    if (slide_step > 0)
    {
        voice->state.generic.filter_slide = 1;
    }
    else if (slide_step < 0)
    {
        voice->state.generic.filter_slide = -1;
    }
    else
    {
        voice->state.generic.filter = voice->state.generic.filter_slide_target;
    }
    return;
}


