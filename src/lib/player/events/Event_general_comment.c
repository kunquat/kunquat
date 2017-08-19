

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_general_decl.h>

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/General_state.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_general_comment_process(General_state* gstate, const Event_params* params)
{
    rassert(gstate != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);

    return params->arg->type == VALUE_TYPE_STRING;
}


