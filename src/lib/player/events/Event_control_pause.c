

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_control_decl.h>

#include <debug/assert.h>
#include <player/Channel.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/General_state.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_control_pause_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    ignore(params);

    global_state->pause = true;
    return true;
}


bool Event_control_resume_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    ignore(params);

    global_state->pause = false;
    return true;
}


