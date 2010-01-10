

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
#include <Event_global_set_jump_row.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_jump_row_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .range.Reltime_type = { { 0, 0 }, { INT64_MAX, KQT_RELTIME_BEAT - 1 } }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_global_set_jump_row,
                                 EVENT_GLOBAL_SET_JUMP_ROW,
                                 row)


static void Event_global_set_jump_row_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_set_jump_row,
                         EVENT_GLOBAL_SET_JUMP_ROW,
                         set_jump_row_desc,
                         Reltime_set(&event->row, 0, 0))


static void Event_global_set_jump_row_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SET_JUMP_ROW);
    assert(play != NULL);
    Event_global_set_jump_row* set_jump_row = (Event_global_set_jump_row*)event;
    Reltime_copy(&play->jump_set_row, &set_jump_row->row);
    return;
}


