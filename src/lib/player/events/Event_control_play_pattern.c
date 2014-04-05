

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <player/Event_type.h>
#include <player/events/Event_common.h>
#include <player/events/Event_control_decl.h>
#include <Value.h>


bool Event_control_play_pattern_process(General_state* gstate, const Value* value)
{
    assert(gstate != NULL);
    assert(value != NULL);
    assert(value->type == VALUE_TYPE_PAT_INST_REF);

    if (!gstate->global)
        return false;

    return false;

#if 0
    Playdata* global_state = (Playdata*)gstate;
    global_state->piref = value->value.Pat_inst_ref_type;
    global_state->mode = PLAY_PATTERN;
    Tstamp_set(&global_state->pos, 0, 0);
    return true;
#endif
}


