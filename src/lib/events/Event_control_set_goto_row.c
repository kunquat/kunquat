

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <Event_common.h>
#include <Event_control.h>
#include <Event_control_set_goto_row.h>
#include <General_state.h>
#include <Playdata.h>
#include <Value.h>
#include <xassert.h>


#if 0
static Event_field_desc set_goto_row_desc[] =
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
#endif


Event_create_constructor(Event_control,
                         EVENT_CONTROL_SET_GOTO_ROW,
                         set_goto_row);


bool Event_control_set_goto_row_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    if (!gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    Reltime_copy(&global_state->goto_set_row, &value->value.Timestamp_type);
    return true;
}


