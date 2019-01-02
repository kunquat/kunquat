

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
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
#include <player/events/Event_common.h>
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

    Voice_group* vgroup = Event_get_voice_group(channel);
    if (vgroup == NULL)
        return NULL;

    for (int i = 0; i < Voice_group_get_size(vgroup); ++i)
    {
        Voice* voice = Voice_group_get_voice(vgroup, i);
        rassert(voice->proc != NULL);
        if (voice->proc->index != proc_index)
            continue;

        Voice_state* vstate = voice->state;
        if (vstate->proc_type == Proc_type_stream)
            return vstate;
    }

    return NULL;
}


