

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Envgen_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_envgen.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <stdlib.h>
#include <string.h>


typedef struct Envgen_vstate
{
    Voice_state parent;

    Time_env_state env_state;
} Envgen_vstate;


int32_t Envgen_vstate_get_size(void)
{
    return sizeof(Envgen_vstate);
}


enum
{
    PORT_IN_STRETCH = 0,
    PORT_IN_COUNT,
};

enum
{
    PORT_OUT_ENV = 0,
    PORT_OUT_COUNT
};


static const int ENVGEN_WB_FIXED_STRETCH = WORK_BUFFER_IMPL_1;


static int32_t Envgen_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_envgen* egen = (Proc_envgen*)proc_state->parent.device->dimpl;
    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    // Get time stretch input
    Work_buffer* stretch_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_STRETCH);
    if (stretch_wb == NULL)
    {
        stretch_wb = Work_buffers_get_buffer_mut(wbs, ENVGEN_WB_FIXED_STRETCH);
        float* stretches = Work_buffer_get_contents_mut(stretch_wb);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            stretches[i] = 0;
        Work_buffer_set_const_start(stretch_wb, buf_start);
    }
    else
    {
        Proc_clamp_pitch_values(stretch_wb, buf_start, buf_stop);
    }

    // Get output buffer for writing
    Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_ENV);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return buf_start;
    }
    float* out_buffer = Work_buffer_get_contents_mut(out_wb);

    const bool is_time_env_enabled =
        egen->is_time_env_enabled && (egen->time_env != NULL);

    const double range_width = egen->y_max - egen->y_min;

    int32_t new_buf_stop = buf_stop;

    int32_t const_start = buf_start;

    if (is_time_env_enabled)
    {
        if (egen->is_release_env && vstate->note_on)
        {
            // Apply the start of release envelope during note on
            const double* first_node = Envelope_get_node(egen->time_env, 0);
            const float first_val = (float)first_node[1];

            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] = first_val;

            const_start = buf_start;
        }
        else
        {
            // Apply the time envelope
            const int32_t env_stop = Time_env_state_process(
                    &egen_state->env_state,
                    egen->time_env,
                    egen->is_loop_enabled,
                    0, // sustain
                    0, 1, // range, NOTE: this needs to be mapped to our [y_min, y_max]!
                    stretch_wb,
                    Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV),
                    buf_start,
                    new_buf_stop,
                    proc_state->parent.audio_rate);

            float* time_env =
                Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV);

            // Check the end of envelope processing
            if (egen_state->env_state.is_finished)
            {
                const double* last_node = Envelope_get_node(
                        egen->time_env, Envelope_node_count(egen->time_env) - 1);
                const float last_value = (float)last_node[1];
                /*
                if (fabs(egen->y_min + last_value * range_width) < 0.0001)
                {
                    Voice_state_set_finished(vstate);
                    new_buf_stop = env_stop;
                }
                else
                // */
                {
                    // Fill the rest of the envelope buffer with the last value
                    for (int32_t i = env_stop; i < new_buf_stop; ++i)
                        time_env[i] = last_value;
                }
            }

            const_start = env_stop;

            // Write to signal output
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] = time_env[i];
        }
    }
    else
    {
        // Initialise with default values
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buffer[i] = 1;
    }

    if (egen->is_linear_force)
    {
        // Convert to dB
        const double global_adjust = egen->global_adjust;

        const int32_t fast_stop = min(const_start, new_buf_stop);

        for (int32_t i = buf_start; i < fast_stop; ++i)
            out_buffer[i] = (float)(fast_scale_to_dB(out_buffer[i]) + global_adjust);

        if (fast_stop < new_buf_stop)
        {
            const float dB =
                (float)(scale_to_dB(out_buffer[fast_stop]) + global_adjust);
            for (int32_t i = fast_stop; i < new_buf_stop; ++i)
                out_buffer[i] = dB;
        }
    }
    else
    {
        if ((egen->y_min != 0) || (egen->y_max != 1))
        {
            // Apply range
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] =
                    (float)(egen->y_min + out_buffer[i] * range_width);
        }

        const float global_adjust = (float)egen->global_adjust;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buffer[i] += global_adjust;
    }

    // Mark constant region of the buffer
    Work_buffer_set_const_start(out_wb, const_start);

    return new_buf_stop;
}


void Envgen_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Envgen_vstate_render_voice;

    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    Time_env_state_init(&egen_state->env_state);

    return;
}


