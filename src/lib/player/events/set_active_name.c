

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/set_active_name.h>

#include <debug/assert.h>
#include <player/Active_names.h>
#include <player/General_state.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool set_active_name(General_state* gstate, Active_cat cat, const Value* value)
{
    rassert(gstate != NULL);
    rassert(cat < ACTIVE_CAT_COUNT);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_STRING);

    Active_names_set(gstate->active_names, cat, value->value.string_type);

    return true;
}


