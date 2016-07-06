

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
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
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_COUNT,
};

enum
{
    PORT_OUT_ENV = 0,
    PORT_OUT_COUNT
};


static const int ENVGEN_WB_FIXED_PITCH = WORK_BUFFER_IMPL_1;


static int32_t Envgen_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_envgen* egen = (Proc_envgen*)proc_state->parent.device->dimpl;
    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    // Get pitch input
    Work_buffer* pitches_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_PITCH);
    if (pitches_wb == NULL)
    {
        pitches_wb = Work_buffers_get_buffer_mut(wbs, ENVGEN_WB_FIXED_PITCH);
        float* pitches = Work_buffer_get_contents_mut(pitches_wb);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitches[i] = 0;
        Work_buffer_set_const_start(pitches_wb, buf_start);
    }

    // Get force scales
    Work_buffer* forces_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_FORCE);

    // Get output buffer for writing
    Work_buffer* out_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, PORT_OUT_ENV);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return buf_start;
    }
    float* out_buffer = Work_buffer_get_contents_mut(out_wb);

    const bool is_time_env_enabled =
        egen->is_time_env_enabled && (egen->time_env != NULL);
    const bool is_force_env_enabled =
        egen->is_force_env_enabled && (egen->force_env != NULL);

    const double range_width = egen->y_max - egen->y_min;

    int32_t new_buf_stop = buf_stop;

    int32_t const_start = buf_start;

    if (is_time_env_enabled && (!egen->is_release_env || !vstate->note_on))
    {
        // Apply the time envelope
        const int32_t env_stop = Time_env_state_process(
                &egen_state->env_state,
                egen->time_env,
                egen->is_loop_enabled,
                egen->env_scale_amount,
                egen->env_scale_center,
                0, // sustain
                0, 1, // range, NOTE: this needs to be mapped to our [y_min, y_max]!
                pitches_wb,
                Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV),
                buf_start,
                new_buf_stop,
                proc_state->parent.audio_rate);

        float* time_env = Work_buffers_get_buffer_contents_mut(
                wbs, WORK_BUFFER_TIME_ENV);

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
    else
    {
        // Initialise with default values
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buffer[i] = 1;
    }

    // Apply const start of input force buffer
    if (forces_wb != NULL)
    {
        const int32_t force_const_start = Work_buffer_get_const_start(forces_wb);
        const_start = max(const_start, force_const_start);
    }

    if (egen->is_linear_force)
    {
        if (forces_wb != NULL)
        {
            // Convert input force to linear scale
            Proc_fill_scale_buffer(forces_wb, forces_wb, buf_start, buf_stop);
            const float* force_scales = Work_buffer_get_contents(forces_wb);

            if (is_force_env_enabled)
            {
                // Apply force envelope
                for (int32_t i = buf_start; i < new_buf_stop; ++i)
                {
                    const float force_scale = force_scales[i];

                    const double vol_clamped = min(1, force_scale);
                    const float factor =
                        (float)Envelope_get_value(egen->force_env, vol_clamped);
                    rassert(isfinite(factor));
                    out_buffer[i] *= factor;
                }
            }
            else
            {
                // Apply linear scaling by default
                for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    out_buffer[i] *= force_scales[i];
            }
        }
        else
        {
            if (is_force_env_enabled)
            {
                // Just apply the rightmost force envelope value (as we assume force 0 dB)
                const float factor = (float)Envelope_get_node(
                        egen->force_env, Envelope_node_count(egen->force_env) - 1)[1];
                for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    out_buffer[i] *= factor;
            }
        }

        // Convert to dB
        {
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

        if (forces_wb != NULL)
        {
            const float* force_scales = Work_buffer_get_contents(forces_wb);

            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] += force_scales[i] + global_adjust;
        }
        else
        {
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] += global_adjust;
        }
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


