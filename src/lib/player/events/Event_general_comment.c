

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/events/Event_general_decl.h>
#include <player/General_state.h>
#include <Value.h>


bool Event_general_comment_process(General_state* gstate, Value* value)
{
    assert(gstate != NULL);
    (void)gstate;
    assert(value != NULL);
    return value->type == VALUE_TYPE_STRING;
}


