

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
#include <limits.h>

#include <Event_common.h>
#include <Event_global_slide_tempo.h>
#include <Reltime.h>

#include <xmemory.h>


static Event_field_desc slide_tempo_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 1, 999 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_slide_tempo,
                                   EVENT_GLOBAL_SLIDE_TEMPO,
                                   double, target_tempo)


static void Event_global_slide_tempo_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_slide_tempo,
                         EVENT_GLOBAL_SLIDE_TEMPO,
                         slide_tempo_desc,
                         event->target_tempo = 120)


static void Event_global_slide_tempo_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_TEMPO);
    assert(play != NULL);
    Event_global_slide_tempo* slide_tempo = (Event_global_slide_tempo*)event;
    if (play->tempo == slide_tempo->target_tempo)
    {
        return;
    }
    Reltime_init(&play->tempo_slide_int_left);
    Reltime_copy(&play->tempo_slide_left, &play->tempo_slide_length);
    double rems_total = (double)Reltime_get_beats(&play->tempo_slide_length) * KQT_RELTIME_BEAT
                         + Reltime_get_rem(&play->tempo_slide_length);
    double slices = rems_total / 36756720; // slide updated 24 times per beat
    play->tempo_slide_update = (slide_tempo->target_tempo - play->tempo) / slices;
    play->tempo_slide_target = slide_tempo->target_tempo;
    if (play->tempo_slide_update < 0)
    {
        play->tempo_slide = -1;
    }
    else if (play->tempo_slide_update > 0)
    {
        play->tempo_slide = 1;
    }
    else
    {
        play->tempo_slide = 0;
        play->tempo = slide_tempo->target_tempo;
    }
    return;
}


