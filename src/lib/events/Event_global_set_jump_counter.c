

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
#include <limits.h>

#include <Event_common.h>
#include <Event_global_set_jump_counter.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_jump_counter_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 0, 65535 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_set_jump_counter,
                                   EVENT_GLOBAL_SET_JUMP_COUNTER,
                                   int64_t, counter)


static void Event_global_set_jump_counter_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_jump_counter,
                         EVENT_GLOBAL_SET_JUMP_COUNTER,
                         set_jump_counter_desc,
                         event->counter = 0)


static void Event_global_set_jump_counter_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_JUMP_COUNTER);
    assert(play != NULL);
    Event_global_set_jump_counter* set_jump_counter = (Event_global_set_jump_counter*)event;
    play->jump_set_counter = set_jump_counter->counter;
    return;
}


