

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Gen_state.h>
#include <xassert.h>


void Gen_state_init(
        Gen_state* gs,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(gs != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Device_state_init(&gs->parent, device, audio_rate, audio_buffer_size);

    return;
}


