

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


#include <stdbool.h>

#include <Active_names.h>
#include <Event.h>
#include <Event_type.h>
#include <File_base.h>
#include <General_state.h>
#include <kunquat/limits.h>
#include <set_active_name.h>
#include <Value.h>
#include <xassert.h>


bool set_active_name(General_state* gstate,
                     Active_cat cat,
                     Active_type type,
                     Value* value)
{
    assert(gstate != NULL);
    assert(cat < ACTIVE_CAT_LAST);
    assert(type < ACTIVE_TYPE_LAST);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_STRING);
    Active_names_set(gstate->active_names, cat, type,
                     value->value.string_type);
    return true;
}


