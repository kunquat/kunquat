

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
#include <Event_global_pattern_delay.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc pattern_delay_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_global_pattern_delay,
                                 EVENT_GLOBAL_PATTERN_DELAY,
                                 length)


static void Event_global_pattern_delay_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_pattern_delay,
                         EVENT_GLOBAL_PATTERN_DELAY,
                         pattern_delay_desc,
                         Reltime_init(&event->length))


static void Event_global_pattern_delay_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_PATTERN_DELAY);
    assert(play != NULL);
    Event_global_pattern_delay* pattern_delay = (Event_global_pattern_delay*)event;
    Reltime_copy(&play->delay_left, &pattern_delay->length);
    return;
}


