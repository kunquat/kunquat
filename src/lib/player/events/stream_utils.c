

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/stream_utils.h>

#include <debug/assert.h>
#include <init/devices/Au_streams.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Proc_type.h>
#include <init/Module.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>
#include <player/Voice_pool.h>
#include <string/var_name.h>

#include <stdio.h>
#include <stdlib.h>


Voice_state* get_target_stream_vstate(Channel* channel, const char* stream_name)
{
    rassert(channel != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));

    const Audio_unit* au =
        Module_get_au_from_input(channel->parent.module, channel->au_input);
    if (au == NULL)
        return NULL;

    const Au_streams* streams = Audio_unit_get_streams(au);
    if (streams == NULL)
        return NULL;

    const int proc_index = Au_streams_get_target_proc_index(streams, stream_name);
    if (proc_index < 0)
        return NULL;

    rassert(proc_index < KQT_PROCESSORS_MAX);
    if (channel->fg[proc_index] == NULL)
        return NULL;

    // Check that we still own the Voice
    channel->fg[proc_index] = Voice_pool_get_voice(
            channel->pool, channel->fg[proc_index], channel->fg_id[proc_index]);
    if (channel->fg[proc_index] == NULL)
        return NULL;

    Voice_state* vstate = channel->fg[proc_index]->state;
    if (vstate->proc_type != Proc_type_stream)
        return NULL;

    return vstate;
}


