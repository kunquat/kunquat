

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <debug/assert.h>
#include <player/Gen_state.h>


void Gen_state_init(
        Gen_state* gen_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(gen_state != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Device_state_init(&gen_state->parent, device, audio_rate, audio_buffer_size);

    return;
}


void Gen_state_reset(Gen_state* gen_state)
{
    assert(gen_state != NULL);

    Device_state_reset(&gen_state->parent);

    return;
}


