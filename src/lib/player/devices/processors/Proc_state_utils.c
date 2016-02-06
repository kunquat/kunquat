

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Proc_state_utils.h>

#include <debug/assert.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <stdlib.h>


#define RAMP_ATTACK_TIME (500.0)


Proc_state* new_Proc_state_default(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* pstate = memory_alloc_item(Proc_state);
    if ((pstate == NULL) ||
            !Proc_state_init(pstate, device, audio_rate, audio_buffer_size))
    {
        del_Device_state(&pstate->parent);
        return NULL;
    }

    return pstate;
}


static void get_mixed_audio_buffers(
        Proc_state* proc_state,
        Device_port_type port_type,
        int32_t port_start,
        int32_t port_stop,
        float* bufs[])
{
    assert(proc_state != NULL);
    assert(port_type < DEVICE_PORT_TYPES);
    assert(port_start >= 0);
    assert(port_start < KQT_DEVICE_PORTS_MAX);
    assert(port_stop >= port_start);
    assert(port_stop <= KQT_DEVICE_PORTS_MAX);
    assert(bufs != NULL);

    Device_state* dstate = (Device_state*)proc_state;

    for (int32_t i = 0, port = port_start; port < port_stop; ++i, ++port)
        bufs[i] = Device_state_get_audio_buffer_contents_mut(dstate, port_type, port);

    return;
}


void Proc_state_get_mixed_audio_in_buffers(
        Proc_state* proc_state, int32_t port_start, int32_t port_stop, float* in_bufs[])
{
    assert(proc_state != NULL);
    assert(port_start >= 0);
    assert(port_start < KQT_DEVICE_PORTS_MAX);
    assert(port_stop >= port_start);
    assert(port_stop <= KQT_DEVICE_PORTS_MAX);
    assert(in_bufs != NULL);

    get_mixed_audio_buffers(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, port_start, port_stop, in_bufs);

    return;
}


void Proc_state_get_mixed_audio_out_buffers(
        Proc_state* proc_state,
        int32_t port_start,
        int32_t port_stop,
        float* out_bufs[])
{
    assert(proc_state != NULL);
    assert(port_start >= 0);
    assert(port_start < KQT_DEVICE_PORTS_MAX);
    assert(port_stop >= port_start);
    assert(port_stop <= KQT_DEVICE_PORTS_MAX);
    assert(out_bufs != NULL);

    get_mixed_audio_buffers(
            proc_state, DEVICE_PORT_TYPE_SEND, port_start, port_stop, out_bufs);

    return;
}


static void get_voice_audio_buffers(
        Proc_state* proc_state,
        Device_port_type port_type,
        int32_t port_start,
        int32_t port_stop,
        float* bufs[])
{
    assert(proc_state != NULL);
    assert(port_type < DEVICE_PORT_TYPES);
    assert(port_start >= 0);
    assert(port_start < KQT_DEVICE_PORTS_MAX);
    assert(port_stop >= port_start);
    assert(port_stop <= KQT_DEVICE_PORTS_MAX);
    assert(bufs != NULL);

    for (int32_t i = 0, port = port_start; port < port_stop; ++i, ++port)
        bufs[i] = Proc_state_get_voice_buffer_contents_mut(proc_state, port_type, port);

    return;
}


void Proc_state_get_voice_audio_in_buffers(
        Proc_state* proc_state, int32_t port_start, int32_t port_stop, float* in_bufs[])
{
    assert(proc_state != NULL);
    assert(port_start >= 0);
    assert(port_start < KQT_DEVICE_PORTS_MAX);
    assert(port_stop >= port_start);
    assert(port_stop <= KQT_DEVICE_PORTS_MAX);
    assert(in_bufs != NULL);

    get_voice_audio_buffers(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, port_start, port_stop, in_bufs);

    return;
}


void Proc_state_get_voice_audio_out_buffers(
        Proc_state* proc_state,
        int32_t port_start,
        int32_t port_stop,
        float* out_bufs[])
{
    assert(proc_state != NULL);
    assert(port_start >= 0);
    assert(port_start < KQT_DEVICE_PORTS_MAX);
    assert(port_stop >= port_start);
    assert(port_stop <= KQT_DEVICE_PORTS_MAX);
    assert(out_bufs != NULL);

    get_voice_audio_buffers(
            proc_state, DEVICE_PORT_TYPE_SEND, port_start, port_stop, out_bufs);

    return;
}


void Proc_ramp_attack(
        Voice_state* vstate,
        int buf_count,
        float* out_bufs[buf_count],
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    assert(vstate != NULL);
    assert(buf_count > 0);
    assert(out_bufs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);

    const float start_ramp_attack = vstate->ramp_attack;
    const float inc = RAMP_ATTACK_TIME / audio_rate;

    for (int ch = 0; ch < buf_count; ++ch)
    {
        if (out_bufs[ch] == NULL)
            continue;

        float ramp_attack = start_ramp_attack;

        for (int32_t i = buf_start; (i < buf_stop) && (ramp_attack < 1); ++i)
        {
            out_bufs[ch][i] *= ramp_attack;
            ramp_attack += inc;
        }

        vstate->ramp_attack = ramp_attack;
    }

    return;
}


Cond_work_buffer* Cond_work_buffer_init(
        Cond_work_buffer* cwb, const Work_buffer* wb, float def_value)
{
    assert(cwb != NULL);

    cwb->index_mask = 0;
    cwb->def_value = def_value;
    cwb->wb_contents = &cwb->def_value;

    if (wb != NULL)
    {
        cwb->index_mask = ~(int32_t)0;
        cwb->wb_contents = Work_buffer_get_contents(wb);
    }

    return cwb;
}


