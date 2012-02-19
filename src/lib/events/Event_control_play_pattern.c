

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
#include <Event_common.h>
#include <Event_control.h>
#include <Event_control_play_pattern.h>
#include <Event_type.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <Playdata.h>
#include <Value.h>
#include <xassert.h>


#if 0
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
#endif


Event_create_constructor(Event_control,
                         EVENT_CONTROL_PLAY_PATTERN,
                         play_pattern);


bool Event_control_play_pattern_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_INT || !gstate->global)
    {
        return false;
    }
    Playdata* global_state = (Playdata*)gstate;
    global_state->pattern = value->value.int_type;
    global_state->mode = PLAY_PATTERN;
    Reltime_set(&global_state->pos, 0, 0);
    return true;
}


