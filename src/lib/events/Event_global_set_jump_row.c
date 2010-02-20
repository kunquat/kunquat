

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
#include <Event_global_set_jump_row.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc set_jump_row_desc[] =
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


Event_create_set_reltime_and_get(Event_global_set_jump_row,
                                 EVENT_GLOBAL_SET_JUMP_ROW,
                                 row);


Event_create_constructor(Event_global_set_jump_row,
                         EVENT_GLOBAL_SET_JUMP_ROW,
                         set_jump_row_desc,
                         Reltime_set(&event->row, 0, 0));


bool Event_global_set_jump_row_handle(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, set_jump_row_desc, data, state);
    if (state->error)
    {
        return false;
    }
    Reltime_copy(&global_state->jump_set_row, &data[0].field.Reltime_type);
    return true;
}


