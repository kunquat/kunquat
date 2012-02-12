

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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Env_var.h>
#include <Event_common.h>
#include <Event_general.h>
#include <Event_general_cond.h>
#include <expr.h>
#include <File_base.h>
#include <General_state.h>
#include <Real.h>
#include <Reltime.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc cond_desc[] =
{
    {
        .type = EVENT_FIELD_BOOL
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_general,
                         EVENT_GENERAL_COND,
                         cond);


bool Event_general_cond_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_BOOL)
    {
        return false;
    }
    if (gstate->cond_level_index >= COND_LEVELS_MAX - 1)
    {
        return true;
    }
    gstate->cond_levels[gstate->cond_level_index + 1].evaluated_cond =
            value->value.bool_type;
    return true;
}


