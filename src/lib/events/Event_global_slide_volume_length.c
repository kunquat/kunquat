

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
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_global_slide_volume_length,
                                 EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                                 length)


static void Event_global_slide_volume_length_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_slide_volume_length,
                         EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                         slide_volume_length_desc,
                         Reltime_set(&event->length, 0, 0))


static void Event_global_slide_volume_length_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_VOLUME_LENGTH);
    assert(play != NULL);
    Event_global_slide_volume_length* slide_volume_length = (Event_global_slide_volume_length*)event;
    if (play->volume_slide != 0)
    {
        play->volume_slide_frames = Reltime_toframes(&slide_volume_length->length,
                                                     play->tempo,
                                                     play->freq);
        double volume_dB = log2(play->volume) * 6;
        double target_dB = log2(play->volume_slide_target) * 6;
        double dB_step = (target_dB - volume_dB) / play->volume_slide_frames;
        play->volume_slide_update = exp2(dB_step / 6);
    }
    Reltime_copy(&play->volume_slide_length, &slide_volume_length->length);
    return;
}


