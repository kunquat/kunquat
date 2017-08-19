

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


#include <player/events/Event_control_decl.h>

#include <debug/assert.h>
#include <player/Channel.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/General_state.h>
#include <player/Master_params.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


#define MAX_GOTOS_WITHOUT_AUDIO 1024


bool Event_control_goto_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    ignore(params);

    Master_params* master_params = (Master_params*)global_state;

    // Prevent us from getting stuck with endless gotos
    if (master_params->goto_safety_counter > MAX_GOTOS_WITHOUT_AUDIO)
    {
        global_state->pause = true;
        master_params->do_goto = false;
        return false;
    }

    ++master_params->goto_safety_counter;
    master_params->do_goto = true;

    return true;
}


bool Event_control_set_goto_row_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_TSTAMP);

    Master_params* master_params = (Master_params*)global_state;
    Tstamp_copy(&master_params->goto_target_row, &params->arg->value.Tstamp_type);

    return true;
}


bool Event_control_set_goto_pat_inst_process(
        General_state* global_state, Channel* channel, const Event_params* params)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_PAT_INST_REF);

    Master_params* master_params = (Master_params*)global_state;
    master_params->goto_target_piref = params->arg->value.Pat_inst_ref_type;

    return true;
}


