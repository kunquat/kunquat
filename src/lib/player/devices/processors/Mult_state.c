

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
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
        const Work_buffer* in1_wb,
        const Work_buffer* in2_wb,
        Work_buffer* out_wb,
        int32_t frame_count)
{
    rassert(frame_count > 0);

    if (out_wb == NULL)
        return;

    if (Work_buffer_is_valid(in1_wb, 0) && Work_buffer_is_valid(in2_wb, 0))
    {
        const float* in1_buf = Work_buffer_get_contents(in1_wb, 0);
        const float* in2_buf = Work_buffer_get_contents(in2_wb, 0);

        float* out_buffer = Work_buffer_get_contents_mut(out_wb, 0);

        for (int32_t i = 0; i < frame_count; ++i)
        {
            float in1 = in1_buf[i];
            float in2 = in2_buf[i];

            // Clamp inputs to finite range (so that we don't accidentally produce NaNs)
            in1 = clamp(in1, -FLT_MAX, FLT_MAX);
            in2 = clamp(in2, -FLT_MAX, FLT_MAX);

            out_buffer[i] = in1 * in2;
        }
    }
    else
    {
        Work_buffer_invalidate(out_wb);
    }

    return;
}


static void Mult_pstate_render_mixed(
        Device_state* dstate,
        Device_thread_state* proc_ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(proc_ts != NULL);
    rassert(wbs != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    for (int ch = 0; ch < 2; ++ch)
    {
        // Get inputs
        Work_buffer* in1_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_1_L + ch);

        Work_buffer* in2_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_2_L + ch);

        // Get output
        Work_buffer* out_wb = Device_thread_state_get_mixed_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_SIGNAL_L + ch);

        // Multiply the signals
        multiply_signals(in1_wb, in2_wb, out_wb, frame_count);
    }

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


static bool is_final_zero(const Work_buffer* in_wb)
{
    return (!Work_buffer_is_valid(in_wb, 0) ||
            (Work_buffer_is_final(in_wb, 0) &&
             (Work_buffer_get_const_start(in_wb, 0) == 0) &&
             (Work_buffer_get_contents(in_wb, 0)[0] == 0.0f)));
}


int32_t Mult_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    rassert(proc_state->is_voice_connected_to_mixed == (vstate != NULL));

    bool all_outs_final_zero = true;

    for (int ch = 0; ch < 2; ++ch)
    {
        // Get inputs
        const Work_buffer* in1_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_1_L + ch);
        const Work_buffer* in2_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_SIGNAL_2_L + ch);

        // Get output
        Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
                proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_SIGNAL_L + ch);
        if (out_wb == NULL)
            continue;

        const bool is_out_final_zero = is_final_zero(in1_wb) || is_final_zero(in2_wb);
        if (is_out_final_zero)
        {
            Work_buffer_clear(out_wb, 0, 0, frame_count);
            Work_buffer_set_const_start(out_wb, 0, 0);
            Work_buffer_set_final(out_wb, 0, true);
        }
        else
        {
            all_outs_final_zero = false;
            multiply_signals(in1_wb, in2_wb, out_wb, frame_count);
        }
    }

    if (all_outs_final_zero)
    {
        if (vstate != NULL)
            vstate->active = false;

        return 0;
    }

    return frame_count;
}


