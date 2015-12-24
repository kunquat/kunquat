

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


#include <devices/processors/Proc_ringmod.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <memory.h>
#include <player/devices/Proc_state.h>

#include <math.h>
#include <stdlib.h>


typedef struct Proc_ringmod
{
    Device_impl parent;
} Proc_ringmod;


static bool Proc_ringmod_init(Device_impl* dimpl);

static Proc_state_render_voice_func Ringmod_state_render_voice;

static void Ringmod_state_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);

static void del_Proc_ringmod(Device_impl* dimpl);


Device_impl* new_Proc_ringmod(Processor* proc)
{
    Proc_ringmod* ringmod = memory_alloc_item(Proc_ringmod);
    if (ringmod == NULL)
        return NULL;

    ringmod->parent.device = (Device*)proc;

    Device_impl_register_init(&ringmod->parent, Proc_ringmod_init);
    Device_impl_register_destroy(&ringmod->parent, del_Proc_ringmod);

    return &ringmod->parent;
}


static Device_state* Proc_ringmod_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* proc_state =
        new_Proc_state_default(device, audio_rate, audio_buffer_size);
    if (proc_state == NULL)
        return NULL;

    proc_state->render_voice = Ringmod_state_render_voice;
    proc_state->render_mixed = Ringmod_state_render_mixed;

    return (Device_state*)proc_state;
}


static bool Proc_ringmod_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_ringmod* ringmod = (Proc_ringmod*)dimpl;

    Device_set_state_creator(ringmod->parent.device, Proc_ringmod_create_state);

    return true;
}


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


static int32_t Ringmod_state_render_voice(
        Proc_state* proc_state,
        Voice_state* vstate,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(proc_state != NULL);
    assert(vstate != NULL);
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


static void Ringmod_state_render_mixed(
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


static void del_Proc_ringmod(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ringmod* ringmod = (Proc_ringmod*)dimpl;
    memory_free(ringmod);

    return;
}


