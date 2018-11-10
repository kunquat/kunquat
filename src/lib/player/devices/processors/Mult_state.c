

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Mult_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_mult.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


enum
{
    PORT_IN_SIGNAL_1_L = 0,
    PORT_IN_SIGNAL_1_R,
    PORT_IN_SIGNAL_2_L,
    PORT_IN_SIGNAL_2_R,
    PORT_IN_COUNT,

    PORT_IN_SIGNAL_1_STOP = PORT_IN_SIGNAL_1_R + 1,
    PORT_IN_SIGNAL_2_STOP = PORT_IN_SIGNAL_2_R + 1,
};

enum
{
    PORT_OUT_SIGNAL_L = 0,
    PORT_OUT_SIGNAL_R,
    PORT_OUT_COUNT
};


static void multiply_signals(
        Work_buffer* in1_buffers[2],
        Work_buffer* in2_buffers[2],
        float* out_buffers[2],
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(in1_buffers != NULL);
    rassert(in2_buffers != NULL);
    rassert(out_buffers != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* in1_wb = in1_buffers[ch];
        Work_buffer* in2_wb = in2_buffers[ch];
        float* out_values = out_buffers[ch];

        if ((in1_wb != NULL) && (in2_wb != NULL) && (out_values != NULL))
        {
            float* in1_values = Work_buffer_get_contents_mut(in1_wb, 0);
            float* in2_values = Work_buffer_get_contents_mut(in2_wb, 0);

            // Clamp inputs to finite range (so that we don't accidentally produce NaNs)
            for (int32_t i = buf_start; i < buf_stop; ++i)
                in1_values[i] = clamp(in1_values[i], -FLT_MAX, FLT_MAX);
            for (int32_t i = buf_start; i < buf_stop; ++i)
                in2_values[i] = clamp(in2_values[i], -FLT_MAX, FLT_MAX);

            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_values[i] = in1_values[i] * in2_values[i];
        }
    }

    return;
}


static void Mult_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    // Get inputs
    Work_buffer* in1_buffers[2] =
    {
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_1_L, NULL),
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_1_R, NULL),
    };

    Work_buffer* in2_buffers[2] =
    {
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_2_L, NULL),
        Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_2_R, NULL),
    };

    // Get outputs
    float* out_buffers[2] = { NULL };
    Proc_state_get_mixed_audio_out_buffers(
            proc_ts, PORT_OUT_SIGNAL_L, PORT_OUT_COUNT, out_buffers);

    // Multiply the signals
    multiply_signals(in1_buffers, in2_buffers, out_buffers, buf_start, buf_stop);

    return;
}


Device_state* new_Mult_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    Proc_state* proc_state =
        new_Proc_state_default(device, audio_rate, audio_buffer_size);
    if (proc_state == NULL)
        return NULL;

    proc_state->render_mixed = Mult_pstate_render_mixed;

    return (Device_state*)proc_state;
}


int32_t Mult_vstate_get_size(void)
{
    return 0;
}


static bool is_final_zero(const Work_buffer* in_wb, int32_t buf_start)
{
    rassert(buf_start >= 0);
    return ((in_wb == NULL) ||
            (Work_buffer_is_final(in_wb, 0) &&
             (Work_buffer_get_const_start(in_wb, 0) <= buf_start) &&
             (Work_buffer_get_contents(in_wb, 0)[buf_start] == 0.0f)));
}


int32_t Mult_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate == NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    // Get inputs
    Work_buffer* in1_buffers[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_1_L, NULL),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_1_R, NULL),
    };

    Work_buffer* in2_buffers[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_2_L, NULL),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_2_R, NULL),
    };

    const bool is_out1_final_zero = (is_final_zero(in1_buffers[0], buf_start) ||
            is_final_zero(in2_buffers[0], buf_start));
    const bool is_out2_final_zero = (is_final_zero(in1_buffers[1], buf_start) ||
            is_final_zero(in2_buffers[1], buf_start));

    if (is_out1_final_zero && is_out2_final_zero)
        return buf_start;

    // Get outputs
    Work_buffer* out_wbs[2] =
    {
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_SIGNAL_L, NULL),
        Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_SIGNAL_R, NULL),
    };

    float* out_buffers[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_ts, PORT_OUT_SIGNAL_L, PORT_OUT_COUNT, out_buffers);

    // Multiply the signals
    multiply_signals(in1_buffers, in2_buffers, out_buffers, buf_start, buf_stop);

    if (is_out1_final_zero && (out_wbs[0] != NULL))
    {
        rassert(out_buffers[0][buf_start] == 0);
        Work_buffer_set_const_start(out_wbs[0], 0, buf_start);
        Work_buffer_set_final(out_wbs[0], 0, true);
    }

    if (is_out2_final_zero && (out_wbs[1] != NULL))
    {
        rassert(out_buffers[1][buf_start] == 0);
        Work_buffer_set_const_start(out_wbs[1], 0, buf_start);
        Work_buffer_set_final(out_wbs[1], 0, true);
    }

    return buf_stop;
}


