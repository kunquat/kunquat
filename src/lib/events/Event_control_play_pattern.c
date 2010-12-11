

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
#include <stdbool.h>

#include <Event.h>
#include <Event_control_play_pattern.h>
#include <Event_type.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <Playdata.h>
#include <xassert.h>


static Event_field_desc play_pattern_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .min.field.integral_type = 0,
        .max.field.integral_type = KQT_PATTERNS_MAX - 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


bool Event_control_play_pattern_process(General_state* gstate, char* fields)
{
    assert(gstate != NULL);
    if (fields == NULL || !gstate->global)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, play_pattern_desc, data, state);
    if (state->error)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->pattern = data[0].field.integral_type;
    global_state->mode = PLAY_PATTERN;
    Reltime_set(&global_state->pos, 0, 0);
    return true;
}


