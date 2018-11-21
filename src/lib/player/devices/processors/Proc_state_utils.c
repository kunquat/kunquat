

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
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
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <stdlib.h>


#define RAMP_ATTACK_TIME (500.0)


Proc_state* new_Proc_state_default(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

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
        Device_thread_state* proc_ts,
        Device_port_type port_type,
        int32_t port_start,
        int32_t port_stop,
        float* bufs[])
{
    rassert(proc_ts != NULL);
    rassert(port_type < DEVICE_PORT_TYPES);
    rassert(port_start >= 0);
    rassert(port_start < KQT_DEVICE_PORTS_MAX);
    rassert(port_stop >= port_start);
    rassert(port_stop <= KQT_DEVICE_PORTS_MAX);
    rassert(bufs != NULL);

    for (int32_t i = 0, port = port_start; port < port_stop; ++i, ++port)
    {
        Work_buffer* wb =
            Device_thread_state_get_mixed_buffer(proc_ts, port_type, port, NULL);
        if ((wb != NULL) &&
                implies(port_type == DEVICE_PORT_TYPE_RECV, Work_buffer_is_valid(wb)))
        {
            rassert(Work_buffer_get_sub_count(wb) == 1);
            bufs[i] = Work_buffer_get_contents_mut(wb, 0);
        }
        else
        {
            bufs[i] = NULL;
        }
    }

    return;
}


void Proc_state_get_mixed_audio_in_buffers(
        Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* in_bufs[])
{
    rassert(proc_ts != NULL);
    rassert(port_start >= 0);
    rassert(port_start < KQT_DEVICE_PORTS_MAX);
    rassert(port_stop >= port_start);
    rassert(port_stop <= KQT_DEVICE_PORTS_MAX);
    rassert(in_bufs != NULL);

    get_mixed_audio_buffers(
            proc_ts, DEVICE_PORT_TYPE_RECV, port_start, port_stop, in_bufs);

    return;
}


void Proc_state_get_mixed_audio_out_buffers(
        Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* out_bufs[])
{
    rassert(proc_ts != NULL);
    rassert(port_start >= 0);
    rassert(port_start < KQT_DEVICE_PORTS_MAX);
    rassert(port_stop >= port_start);
    rassert(port_stop <= KQT_DEVICE_PORTS_MAX);
    rassert(out_bufs != NULL);

    get_mixed_audio_buffers(
            proc_ts, DEVICE_PORT_TYPE_SEND, port_start, port_stop, out_bufs);

    return;
}


static void get_voice_audio_buffers(
        const Device_thread_state* proc_ts,
        Device_port_type port_type,
        int32_t port_start,
        int32_t port_stop,
        float* bufs[])
{
    rassert(proc_ts != NULL);
    rassert(port_type < DEVICE_PORT_TYPES);
    rassert(port_start >= 0);
    rassert(port_start < KQT_DEVICE_PORTS_MAX);
    rassert(port_stop >= port_start);
    rassert(port_stop <= KQT_DEVICE_PORTS_MAX);
    rassert(bufs != NULL);

    for (int32_t i = 0, port = port_start; port < port_stop; ++i, ++port)
    {
        Work_buffer* wb =
            Device_thread_state_get_voice_buffer(proc_ts, port_type, port, NULL);
        if (wb != NULL)
        {
            rassert(Work_buffer_get_sub_count(wb) == 1);
            bufs[i] = Work_buffer_get_contents_mut(wb, 0);
        }
        else
        {
            bufs[i] = NULL;
        }
    }

    return;
}


void Proc_state_get_voice_audio_in_buffers(
        const Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* in_bufs[])
{
    rassert(proc_ts != NULL);
    rassert(port_start >= 0);
    rassert(port_start < KQT_DEVICE_PORTS_MAX);
    rassert(port_stop >= port_start);
    rassert(port_stop <= KQT_DEVICE_PORTS_MAX);
    rassert(in_bufs != NULL);

    get_voice_audio_buffers(
            proc_ts, DEVICE_PORT_TYPE_RECV, port_start, port_stop, in_bufs);

    return;
}


void Proc_state_get_voice_audio_out_buffers(
        const Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* out_bufs[])
{
    rassert(proc_ts != NULL);
    rassert(port_start >= 0);
    rassert(port_start < KQT_DEVICE_PORTS_MAX);
    rassert(port_stop >= port_start);
    rassert(port_stop <= KQT_DEVICE_PORTS_MAX);
    rassert(out_bufs != NULL);

    get_voice_audio_buffers(
            proc_ts, DEVICE_PORT_TYPE_SEND, port_start, port_stop, out_bufs);

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
    rassert(vstate != NULL);
    rassert(buf_count > 0);
    rassert(out_bufs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(audio_rate > 0);

    const float start_ramp_attack = (float)vstate->ramp_attack;
    const float inc = (float)(RAMP_ATTACK_TIME / audio_rate);

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


void Proc_fill_freq_buffer(
        Work_buffer* freqs,
        Work_buffer* pitches,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(freqs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    if ((pitches != NULL) && Work_buffer_is_valid(pitches))
    {
        Proc_clamp_pitch_values(pitches, buf_start, buf_stop);

        const int32_t const_start = Work_buffer_get_const_start(pitches, 0);
        float* freqs_data = Work_buffer_get_contents_mut(freqs, 0);

        float* pitches_data = Work_buffer_get_contents_mut(pitches, 0);

        const int32_t fast_stop = clamp(const_start, buf_start, buf_stop);

        for (int32_t i = buf_start; i < fast_stop; ++i)
            freqs_data[i] = (float)fast_cents_to_Hz(pitches_data[i]);

        //fprintf(stdout, "%d %d %d\n", (int)buf_start, (int)fast_stop, (int)buf_stop);

        if (fast_stop < buf_stop)
        {
            const float pitch = pitches_data[fast_stop];
            const float freq = isfinite(pitch) ? (float)cents_to_Hz(pitch) : 0.0f;
            for (int32_t i = fast_stop; i < buf_stop; ++i)
                freqs_data[i] = freq;
        }

        Work_buffer_set_const_start(freqs, 0, const_start);
    }
    else
    {
        float* freqs_data = Work_buffer_get_contents_mut(freqs, 0);

        for (int32_t i = buf_start; i < buf_stop; ++i)
            freqs_data[i] = 440;

        Work_buffer_set_const_start(freqs, 0, buf_start);
    }

    return;
}


void Proc_fill_scale_buffer(
        Work_buffer* scales,
        Work_buffer* dBs,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(scales != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    if ((dBs != NULL) && Work_buffer_is_valid(dBs))
    {
        const int32_t const_start = Work_buffer_get_const_start(dBs, 0);
        float* scales_data = Work_buffer_get_contents_mut(scales, 0);

        float* dBs_data = Work_buffer_get_contents_mut(dBs, 0);

        const int32_t fast_stop = clamp(const_start, buf_start, buf_stop);

        // Sanitise input values
        {
            const float bound = 10000.0f;

            for (int32_t i = buf_start; i < fast_stop; ++i)
                dBs_data[i] = clamp(dBs_data[i], -bound, bound);

            if (fast_stop < buf_stop)
            {
                const float dB = clamp(dBs_data[fast_stop], -bound, bound);
                for (int32_t i = fast_stop; i < buf_stop; ++i)
                    dBs_data[i] = dB;
            }
        }

        for (int32_t i = buf_start; i < fast_stop; ++i)
            scales_data[i] = (float)fast_dB_to_scale(dBs_data[i]);

        //fprintf(stdout, "%d %d %d\n", (int)buf_start, (int)fast_stop, (int)buf_stop);

        if (fast_stop < buf_stop)
        {
            const float scale = (float)dB_to_scale(dBs_data[fast_stop]);
            for (int32_t i = fast_stop; i < buf_stop; ++i)
                scales_data[i] = scale;
        }

        Work_buffer_set_const_start(scales, 0, const_start);
    }
    else
    {
        float* scales_data = Work_buffer_get_contents_mut(scales, 0);

        for (int32_t i = buf_start; i < buf_stop; ++i)
            scales_data[i] = 1;

        Work_buffer_set_const_start(scales, 0, buf_start);
    }

    return;
}


void Proc_clamp_pitch_values(Work_buffer* pitches, int32_t buf_start, int32_t buf_stop)
{
    rassert(pitches != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    const int32_t const_start = Work_buffer_get_const_start(pitches, 0);
    const int32_t fast_stop = clamp(const_start, buf_start, buf_stop);

    const float bound = 2000000.0f;

    float* pitches_data = Work_buffer_get_contents_mut(pitches, 0);

    for (int32_t i = buf_start; i < fast_stop; ++i)
        pitches_data[i] = clamp(pitches_data[i], -bound, bound);

    if (fast_stop < buf_stop)
    {
        const float pitch = clamp(pitches_data[fast_stop], -bound, bound);
        for (int32_t i = fast_stop; i < buf_stop; ++i)
            pitches_data[i] = pitch;
    }

    Work_buffer_set_const_start(pitches, 0, const_start);

    return;
}


Cond_work_buffer* Cond_work_buffer_init(
        Cond_work_buffer* cwb, const Work_buffer* wb, float def_value)
{
    rassert(cwb != NULL);

    cwb->index_mask = 0;
    cwb->def_value = def_value;
    cwb->wb_contents = &cwb->def_value;

    if ((wb != NULL) && Work_buffer_is_valid(wb))
    {
        cwb->index_mask = ~(int32_t)0;
        cwb->wb_contents = Work_buffer_get_contents(wb, 0);
    }

    return cwb;
}


