

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
#include <Event_global_pattern_delay.h>
#include <File_base.h>
#include <kunquat/limits.h>

#include <xmemory.h>


static Event_field_desc pattern_delay_desc[] =
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


Event_create_set_reltime_and_get(Event_global_pattern_delay,
                                 EVENT_GLOBAL_PATTERN_DELAY,
                                 length);


Event_create_constructor(Event_global_pattern_delay,
                         EVENT_GLOBAL_PATTERN_DELAY,
                         pattern_delay_desc,
                         Reltime_init(&event->length));


bool Event_global_pattern_delay_process(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field delay[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, pattern_delay_desc, delay, state);
    if (state->error)
    {
        return false;
    }
    Reltime_copy(&global_state->delay_left, &delay[0].field.Reltime_type);
    return true;
}


