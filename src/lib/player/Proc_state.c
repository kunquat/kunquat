

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


#include <stdbool.h>

#include <debug/assert.h>
#include <player/Proc_state.h>


bool Proc_state_init(
        Proc_state* proc_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(proc_state != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    proc_state->in_voice_buf = NULL;
    proc_state->out_voice_buf = NULL;

    Device_state_init(&proc_state->parent, device, audio_rate, audio_buffer_size);

    proc_state->in_voice_buf = new_Audio_buffer(audio_buffer_size);
    proc_state->out_voice_buf = new_Audio_buffer(audio_buffer_size);
    if ((proc_state->in_voice_buf == NULL) || (proc_state->out_voice_buf == NULL))
    {
        Proc_state_deinit(&proc_state->parent);
        return false;
    }

    proc_state->parent.resize_buffers = Proc_state_resize_buffers;
    proc_state->parent.deinit = Proc_state_deinit;

    return true;
}


void Proc_state_reset(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    Device_state_reset(&proc_state->parent);

    return;
}


Audio_buffer* Proc_state_get_input_voice_buffer(Proc_state* proc_state)
{
    assert(proc_state != NULL);
    return proc_state->in_voice_buf;
}


Audio_buffer* Proc_state_get_output_voice_buffer(Proc_state* proc_state)
{
    assert(proc_state != NULL);
    return proc_state->out_voice_buf;
}


bool Proc_state_resize_buffers(Device_state* dstate, int32_t new_size)
{
    assert(dstate != NULL);
    assert(new_size >= 0);

    Proc_state* proc_state = (Proc_state*)dstate;

    if (!Audio_buffer_resize(proc_state->in_voice_buf, new_size))
        return false;
    if (!Audio_buffer_resize(proc_state->out_voice_buf, new_size))
        return false;

    return true;
}


void Proc_state_deinit(Device_state* dstate)
{
    assert(dstate != NULL);

    Proc_state* proc_state = (Proc_state*)dstate;

    del_Audio_buffer(proc_state->out_voice_buf);
    proc_state->out_voice_buf = NULL;
    del_Audio_buffer(proc_state->in_voice_buf);
    proc_state->in_voice_buf = NULL;

    return;
}


