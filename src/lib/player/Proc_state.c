

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


#include <debug/assert.h>
#include <player/Proc_state.h>


void Proc_state_init(
        Proc_state* proc_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(proc_state != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Device_state_init(&proc_state->parent, device, audio_rate, audio_buffer_size);

    return;
}


void Proc_state_reset(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    Device_state_reset(&proc_state->parent);

    return;
}


