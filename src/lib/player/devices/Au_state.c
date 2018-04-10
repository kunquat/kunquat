

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Au_state.h>

#include <debug/assert.h>
#include <init/Connections.h>
#include <init/devices/Au_event_map.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Proc_table.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/devices/Device_state.h>
#include <player/devices/Device_thread_state.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


static Device_state_reset_func Au_state_reset;


static void Au_state_fire_event(
        Device_state* dstate, const char* event_name, const Value* arg, Random* rand)
{
    rassert(dstate != NULL);
    rassert(event_name != NULL);
    rassert(arg != NULL);
    rassert(rand != NULL);

    const Audio_unit* au = (const Audio_unit*)dstate->device;

    const Au_event_map* map = Audio_unit_get_event_map(au);
    if (map == NULL)
        return;

    const Proc_table* procs = Audio_unit_get_procs(au);
    Au_event_iter* iter = AU_EVENT_ITER_AUTO;
    Au_event_iter_result* result = Au_event_iter_init(iter, map, event_name, arg, rand);
    while (result != NULL)
    {
        rassert(result->dev_type == AU_EVENT_TARGET_DEV_PROC);
        const Device* device =
            (const Device*)Proc_table_get_proc(procs, result->dev_index);
        if (device != NULL)
        {
            Device_states* dstates = ((Au_state*)dstate)->dstates;
            Device_state* pstate =
                Device_states_get_state(dstates, Device_get_id(device));
            Device_state_fire_event(pstate, result->event_name, &result->arg, rand);
        }

        result = Au_event_iter_get_next(iter);
    }

    return;
}


static bool Au_state_init(
        Au_state* au_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    rassert(au_state != NULL);

    if (!Device_state_init(&au_state->parent, device, audio_rate, audio_buffer_size))
        return false;

    au_state->parent.reset = Au_state_reset;
    au_state->parent.fire_dev_event = Au_state_fire_event;

    au_state->dstates = NULL;

    Au_state_reset(&au_state->parent);

    return true;
}


Device_state* new_Au_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Au_state* au_state = memory_alloc_item(Au_state);
    if (au_state == NULL)
        return NULL;

    if (!Au_state_init(au_state, device, audio_rate, audio_buffer_size))
    {
        del_Device_state(&au_state->parent);
        return NULL;
    }

    return &au_state->parent;
}


void Au_state_set_device_states(Au_state* au_state, Device_states* dstates)
{
    rassert(au_state != NULL);
    rassert(dstates != NULL);

    au_state->dstates = dstates;

    return;
}


void Au_state_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Au_state* au_state = (Au_state*)dstate;
    au_state->bypass = false;
    au_state->sustain = 0.0;

    return;
}


