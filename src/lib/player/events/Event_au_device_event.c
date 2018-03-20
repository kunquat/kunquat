

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_au_decl.h>

#include <debug/assert.h>
#include <init/devices/Au_event_map.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Device.h>
#include <player/Channel.h>
#include <player/Device_states.h>
#include <player/devices/Device_state.h>
#include <player/events/Event_params.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_au_fire_device_event_process(
        const Audio_unit* au,
        const Au_params* au_params,
        Au_state* au_state,
        Master_params* master_params,
        Channel* channel,
        Device_states* dstates,
        const Event_params* params)
{
    rassert(au != NULL);
    rassert(au_params != NULL);
    rassert(au_state != NULL);
    rassert(master_params != NULL);
    rassert(channel != NULL);
    rassert(dstates != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);

    const char* event_name =
        Active_names_get(channel->parent.active_names, ACTIVE_CAT_DEVICE_EVENT);
    rassert(event_name != NULL);

    const Au_event_map* map = Audio_unit_get_event_map(au);
    if (map == NULL)
        return true;

    // Get evaluated arguments for each target
    Proc_table* procs = Audio_unit_get_procs(au);
    Au_event_iter* iter = AU_EVENT_ITER_AUTO;
    Au_event_iter_result* result =
        Au_event_iter_init(iter, map, event_name, params->arg, &channel->rand);
    while (result != NULL)
    {
        const Device* device = NULL;
        if (result->dev_type == AU_EVENT_TARGET_DEV_AU)
        {
            device = (const Device*)Audio_unit_get_au(au, result->dev_index);
        }
        else
        {
            rassert(result->dev_type == AU_EVENT_TARGET_DEV_PROC);
            device = (const Device*)Proc_table_get_proc(procs, result->dev_index);
        }

        if (device != NULL)
        {
            Device_state* dstate =
                Device_states_get_state(dstates, Device_get_id(device));
            Device_state_fire_event(dstate, result->event_name, &result->arg);
        }

        result = Au_event_iter_get_next(iter);
    }

    return true;
}


