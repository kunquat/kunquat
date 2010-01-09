

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

#include <Event_common.h>
#include <Event_global_set_tempo.h>

#include <xmemory.h>


static Event_field_desc set_tempo_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 1, 999 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_set_tempo,
                                   EVENT_GLOBAL_SET_TEMPO,
                                   double, tempo)


static void Event_global_set_tempo_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_tempo,
                         EVENT_GLOBAL_SET_TEMPO,
                         set_tempo_desc,
                         event->tempo = 120)


static void Event_global_set_tempo_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_TEMPO);
    assert(play != NULL);
    Event_global_set_tempo* set_tempo = (Event_global_set_tempo*)event;
    play->tempo = set_tempo->tempo;
    play->tempo_slide = 0;
    return;
}


