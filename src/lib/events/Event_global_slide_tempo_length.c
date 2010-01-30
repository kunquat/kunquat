

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

#include <Event_common.h>
#include <Event_global_slide_tempo_length.h>
#include <Reltime.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc slide_tempo_length_desc[] =
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


Event_create_set_reltime_and_get(Event_global_slide_tempo_length,
                                 EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                                 length)


static void Event_global_slide_tempo_length_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_slide_tempo_length,
                         EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                         slide_tempo_length_desc,
                         Reltime_set(&event->length, 0, 0))


static void Event_global_slide_tempo_length_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_TEMPO_LENGTH);
    assert(play != NULL);
    Event_global_slide_tempo_length* slide_tempo_length = (Event_global_slide_tempo_length*)event;
    if (play->tempo_slide != 0)
    {
        Reltime_init(&play->tempo_slide_int_left);
        Reltime_copy(&play->tempo_slide_left, &slide_tempo_length->length);
        double rems_total = (double)Reltime_get_beats(&slide_tempo_length->length) *
                            KQT_RELTIME_BEAT +
                            Reltime_get_rem(&slide_tempo_length->length);
        double slices = rems_total / 36756720; // slide updated 24 times per beat
        play->tempo_slide_update = (play->tempo_slide_target - play->tempo) / slices;
    }
    Reltime_copy(&play->tempo_slide_length, &slide_tempo_length->length);
    return;
}


