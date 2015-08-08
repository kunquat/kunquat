

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


#include <math.h>
#include <stdlib.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_ringmod.h>
#include <devices/processors/Proc_utils.h>
#include <memory.h>
#include <player/Proc_state.h>


typedef struct Proc_ringmod
{
    Device_impl parent;
} Proc_ringmod;


static bool Proc_ringmod_init(Device_impl* dimpl);

static Proc_process_vstate_func Proc_ringmod_process_vstate;

static void Proc_ringmod_process_signal(
        const Device* device,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
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


static bool Proc_ringmod_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_ringmod* ringmod = (Proc_ringmod*)dimpl;

    Device_set_state_creator(ringmod->parent.device, new_Proc_state_default);

    Processor* proc = (Processor*)ringmod->parent.device;
    proc->process_vstate = Proc_ringmod_process_vstate;

    Device_set_process(ringmod->parent.device, Proc_ringmod_process_signal);

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
        const kqt_frame* in1_values = Audio_buffer_get_buffer(in1_buffer, ch);
        const kqt_frame* in2_values = Audio_buffer_get_buffer(in2_buffer, ch);
        kqt_frame* out_values = Audio_buffer_get_buffer(out_buffer, ch);

        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_values[i] = in1_values[i] * in2_values[i];
    }

    return;
}


static uint32_t Proc_ringmod_process_vstate(
        const Processor* proc,
        Proc_state* proc_state,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

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


static void Proc_ringmod_process_signal(
        const Device* device,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));
    assert(dstate != NULL);

    // Get inputs
    Audio_buffer* in1_buffer = Device_state_get_audio_buffer(
            dstate, DEVICE_PORT_TYPE_RECEIVE, 0);
    Audio_buffer* in2_buffer = Device_state_get_audio_buffer(
            dstate, DEVICE_PORT_TYPE_RECEIVE, 1);
    if ((in1_buffer == NULL) || (in2_buffer == NULL))
        return;

    // Get outputs
    Audio_buffer* out_buffer = Device_state_get_audio_buffer(
            dstate, DEVICE_PORT_TYPE_SEND, 0);
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


