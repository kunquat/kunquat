

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


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <init/devices/Au_event_map.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Device_impl.h>
#include <init/Module.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/events/set_active_name.h>
#include <player/Voice.h>
#include <Value.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


bool Event_channel_set_device_event_name_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_STRING);

    return set_active_name(&ch->parent, ACTIVE_CAT_DEVICE_EVENT, params->arg);
}


bool Event_channel_fire_device_event_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);

    const char* event_name =
        Active_names_get(ch->parent.active_names, ACTIVE_CAT_DEVICE_EVENT);
    rassert(event_name != NULL);

    const Audio_unit* au = Module_get_au_from_input(ch->parent.module, ch->au_input);
    if (au == NULL)
        return true;

    const Au_event_map* map = Audio_unit_get_event_map(au);
    if (map == NULL)
        return true;

    // Get evaluated arguments for each target
    Proc_table* procs = Audio_unit_get_procs(au);
    Au_event_iter* iter = AU_EVENT_ITER_AUTO;
    Au_event_iter_result* result =
        Au_event_iter_init(iter, map, event_name, params->arg, &ch->rand);
    while (result != NULL)
    {
        if (result->dev_type == AU_EVENT_TARGET_DEV_PROC)
        {
            const Processor* proc = Proc_table_get_proc(procs, result->dev_index);
            const Device* device = (const Device*)proc;
            if ((proc != NULL) && (device->dimpl != NULL))
            {
                // Find the Voice associated with our Processor
                Voice* voice = NULL;
                for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
                {
                    Event_check_voice(ch, i);
                    Voice* cur_voice = ch->fg[i];

                    if (cur_voice->proc == proc)
                    {
                        voice = cur_voice;
                        break;
                    }
                }

                if (voice != NULL)
                {
                    // Target Voice found -> fire the event if implemented
                    const Device_impl* dimpl = device->dimpl;
                    if (dimpl->fire_voice_dev_event != NULL)
                    {
                        const Device_state* dstate =
                            Device_states_get_state(dstates, Device_get_id(device));
                        dimpl->fire_voice_dev_event(
                                voice->state, dstate, result->event_name, &result->arg);
                    }
                }
            }
        }

        result = Au_event_iter_get_next(iter);
    }

    return true;
}


