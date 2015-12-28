

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Ringmod_state.h>

#include <debug/assert.h>
#include <devices/processors/Proc_ringmod.h>
#include <memory.h>
#include <player/Audio_buffer.h>
#include <player/devices/processors/Proc_utils.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


static void multiply_signals(
        Audio_buffer* in1_buffer,
        Audio_buffer* in2_buffer,
        Audio_buffer* out_buffer,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(in1_buffer != NULL);
    assert(in2_buffer != NULL);
    assert(out_buffer != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);

    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in1_values = Audio_buffer_get_buffer(in1_buffer, ch);
        const float* in2_values = Audio_buffer_get_buffer(in2_buffer, ch);
        float* out_values = Audio_buffer_get_buffer(out_buffer, ch);

        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_values[i] = in1_values[i] * in2_values[i];
    }

    return;
}


static void Ringmod_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    // Get inputs
    Audio_buffer* in1_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, 0);
    Audio_buffer* in2_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, 1);
    if ((in1_buffer == NULL) || (in2_buffer == NULL))
        return;

    // Get outputs
    Audio_buffer* out_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    // Multiply the signals
    multiply_signals(in1_buffer, in2_buffer, out_buffer, buf_start, buf_stop);

    return;
}


Device_state* new_Ringmod_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* proc_state =
        new_Proc_state_default(device, audio_rate, audio_buffer_size);
    if (proc_state == NULL)
        return NULL;

    proc_state->render_mixed = Ringmod_pstate_render_mixed;

    return (Device_state*)proc_state;
}


static int32_t Ringmod_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    // Get inputs
    Audio_buffer* in1_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 0);
    Audio_buffer* in2_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 1);
    if ((in1_buffer == NULL) || (in2_buffer == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    // Multiply the signals
    multiply_signals(in1_buffer, in2_buffer, out_buffer, buf_start, buf_stop);

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Ringmod_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Ringmod_vstate_render_voice;

    return;
}


