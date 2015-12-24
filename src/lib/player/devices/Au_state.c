

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
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
#include <player/devices/Device_state.h>

#include <stdbool.h>
#include <stdlib.h>


static Device_state_reset_func Au_state_reset;


void Au_state_init(
        Au_state* au_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(au_state != NULL);

    Device_state_init(&au_state->parent, device, audio_rate, audio_buffer_size);
    au_state->parent.reset = Au_state_reset;

    au_state->dstates = NULL;

    Au_state_reset(&au_state->parent);

    return;
}


void Au_state_set_device_states(Au_state* au_state, Device_states* dstates)
{
    assert(au_state != NULL);
    assert(dstates != NULL);

    au_state->dstates = dstates;

    return;
}


void Au_state_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Au_state* au_state = (Au_state*)dstate;
    au_state->bypass = false;
    au_state->sustain = 0.0;

    return;
}


