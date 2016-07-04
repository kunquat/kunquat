

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
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
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <player/Event_type.h>
#include <player/events/Event_common.h>
#include <player/Master_params.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_control_play_pattern_process(
        General_state* global_state, Channel* channel, const Value* value)
{
    rassert(global_state != NULL);
    rassert(channel != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_PAT_INST_REF);

    Master_params* master_params = (Master_params*)global_state;
    master_params->playback_state = PLAYBACK_PATTERN;
    master_params->pattern_playback_flag = true;

    master_params->cur_pos.track = -1;
    master_params->cur_pos.system = -1;
    Tstamp_set(&master_params->cur_pos.pat_pos, 0, 0);
    master_params->cur_pos.piref = value->value.Pat_inst_ref_type;

    // Reset parameters to make sure we start at the new location immediately
    master_params->parent.pause = false;
    Tstamp_set(&master_params->delay_left, 0, 0);

    return true;
}


