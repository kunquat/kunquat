

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


void Mult_get_port_groups(
        const Device_impl* dimpl, Device_port_type port_type, Device_port_groups groups)
{
    rassert(dimpl != NULL);
    rassert(groups != NULL);

    switch (port_type)
    {
        case DEVICE_PORT_TYPE_RECV: Device_port_groups_init(groups, 2, 2, 0); break;

        case DEVICE_PORT_TYPE_SEND: Device_port_groups_init(groups, 2, 0); break;

        default:
            rassert(false);
    }

    return;
}


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
        Work_buffer* in1_wb,
        Work_buffer* in2_wb,
        float* out_buffer,
        int32_t frame_count)
{
    rassert(out_buffer != NULL);
    rassert(frame_count > 0);

    const int32_t item_count = frame_count * 2;

    if ((in1_wb != NULL) && (in2_wb != NULL))
    {
        const float* in1_buf = Work_buffer_get_contents(in1_wb, 0);
        const float* in2_buf = Work_buffer_get_contents(in2_wb, 0);

        for (int32_t i = 0; i < item_count; ++i)
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
        for (int32_t i = 0; i < item_count; ++i)
            out_buffer[i] = 0;
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

    // Get inputs
    Work_buffer* in1_wb =
        Proc_get_mixed_input_2ch(proc_ts, PORT_IN_SIGNAL_1_L, frame_count);

    Work_buffer* in2_wb =
        Proc_get_mixed_input_2ch(proc_ts, PORT_IN_SIGNAL_2_L, frame_count);

    // Get output
    Work_buffer* out_wb = Proc_get_mixed_output_2ch(proc_ts, PORT_OUT_SIGNAL_L);
    rassert(out_wb != NULL);
    float* out_buf = Work_buffer_get_contents_mut(out_wb, 0);

    // Multiply the signals
    multiply_signals(in1_wb, in2_wb, out_buf, frame_count);

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


static bool is_final_zero(const Work_buffer* in_wb, int ch)
{
    return ((in_wb == NULL) ||
            (Work_buffer_is_final(in_wb, ch) &&
             (Work_buffer_get_const_start(in_wb, ch) == 0) &&
             (Work_buffer_get_contents(in_wb, ch)[0] == 0.0f)));
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
    rassert(vstate == NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    // Get inputs
    Work_buffer* in1_wb =
        Proc_get_voice_input_2ch(proc_ts, PORT_IN_SIGNAL_1_L, frame_count);

    Work_buffer* in2_wb =
        Proc_get_voice_input_2ch(proc_ts, PORT_IN_SIGNAL_2_L, frame_count);

    const bool is_out1_final_zero =
        (is_final_zero(in1_wb, 0) || is_final_zero(in2_wb, 0));
    const bool is_out2_final_zero =
        (is_final_zero(in1_wb, 1) || is_final_zero(in2_wb, 1));

    if (is_out1_final_zero && is_out2_final_zero)
        return 0;

    // Get outputs
    Work_buffer* out_wb = Proc_get_voice_output_2ch(proc_ts, PORT_OUT_SIGNAL_L);
    rassert(out_wb != NULL);
    float* out_buf = Work_buffer_get_contents_mut(out_wb, 0);

    // Multiply the signals
    multiply_signals(in1_wb, in2_wb, out_buf, frame_count);

    if (is_out1_final_zero)
    {
        Work_buffer_set_const_start(out_wb, 0, 0);
        Work_buffer_set_final(out_wb, 0, true);
    }

    if (is_out2_final_zero)
    {
        Work_buffer_set_const_start(out_wb, 1, 0);
        Work_buffer_set_final(out_wb, 1, true);
    }

    return frame_count;
}


