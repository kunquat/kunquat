

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdbool.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <player/Active_names.h>
#include <player/Event_type.h>
#include <player/events/set_active_name.h>
#include <player/General_state.h>
#include <Value.h>


bool set_active_name(
        General_state* gstate,
        Active_cat cat,
        Active_type type,
        const Value* value)
{
    assert(gstate != NULL);
    assert(cat < ACTIVE_CAT_COUNT);
    assert(type < ACTIVE_TYPE_COUNT);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);

    Active_names_set(gstate->active_names, cat, type, value->value.string_type);

    return true;
}


