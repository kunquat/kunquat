

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
#include <player/DSP_state.h>


void DSP_state_init(
        DSP_state* dsp_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(dsp_state != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Device_state_init(&dsp_state->parent, device, audio_rate, audio_buffer_size);

    return;
}


void DSP_state_reset(DSP_state* dsp_state)
{
    assert(dsp_state != NULL);

    Device_state_reset(&dsp_state->parent);

    return;
}


