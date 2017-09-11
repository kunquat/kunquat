

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2017
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
#include <init/devices/Audio_unit.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/devices/Device_state.h>
#include <player/devices/Device_thread_state.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


static Device_state_reset_func Au_state_reset;


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


