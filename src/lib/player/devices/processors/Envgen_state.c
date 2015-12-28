

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


#include <player/devices/processors/Envgen_state.h>

#include <debug/assert.h>
#include <devices/processors/Proc_envgen.h>
#include <devices/processors/Proc_utils.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <mathnum/common.h>
#include <memory.h>

#include <stdlib.h>
#include <string.h>


typedef struct Envgen_vstate
{
    Voice_state parent;

    Time_env_state env_state;
} Envgen_vstate;


size_t Envgen_vstate_get_size(void)
{
    return sizeof(Envgen_vstate);
}


static int32_t Envgen_vstate_render_voice(
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

    const Processor* proc = (const Processor*)proc_state->parent.device;
    const Proc_envgen* egen = (Proc_envgen*)proc_state->parent.device->dimpl;
    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    // Get output buffer for writing
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);
    float* audio_l = Audio_buffer_get_buffer(out_buffer, 0);
    float* audio_r = Audio_buffer_get_buffer(out_buffer, 1);

    const bool is_time_env_enabled =
        egen->is_time_env_enabled && (egen->time_env != NULL);
    const bool is_force_env_enabled =
        egen->is_force_env_enabled && (egen->force_env != NULL);

    const double range_width = egen->y_max - egen->y_min;

    int32_t new_buf_stop = buf_stop;

    if (is_time_env_enabled)
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
                Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH),
                wbs,
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
            const double last_value = last_node[1];
            if (fabs(egen->y_min + last_value * range_width) < 0.0001)
            {
                vstate->active = false;
                new_buf_stop = env_stop;
            }
            else
            {
                // Fill the rest of the envelope buffer with the last value
                for (int32_t i = env_stop; i < new_buf_stop; ++i)
                    time_env[i] = last_value;
            }
        }

        // Write to audio output
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] = time_env[i];
    }
    else
    {
        // Initialise with default values
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] = 1;
    }

    // Apply range
    if ((egen->y_min != 0) || (egen->y_max != 1))
    {
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] = egen->y_min + audio_l[i] * range_width;
    }

    // Apply our internal scaling
    if (egen->scale != 1)
    {
        const double scale = egen->scale;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] *= scale;
    }

    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE))
    {
        const float* actual_forces = Work_buffers_get_buffer_contents(
                wbs, WORK_BUFFER_ACTUAL_FORCES);

        if (is_force_env_enabled)
        {
            // Apply force envelope
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
            {
                const float actual_force = actual_forces[i];

                const double force_clamped = min(1, actual_force);
                const double factor = Envelope_get_value(egen->force_env, force_clamped);
                assert(isfinite(factor));
                audio_l[i] *= factor;
            }
        }
        else
        {
            // Apply linear scaling by default
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                audio_l[i] *= actual_forces[i];
        }
    }
    else
    {
        if (is_force_env_enabled)
        {
            // Just apply the rightmost force envelope value (as we assume force 1)
            const double factor = Envelope_get_node(
                    egen->force_env, Envelope_node_count(egen->force_env) - 1)[1];
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                audio_l[i] *= factor;
        }
    }

    // Copy to the right channel
    {
        const int32_t frame_count = new_buf_stop - buf_start;
        assert(frame_count >= 0);
        memcpy(audio_r + buf_start, audio_l + buf_start, sizeof(float) * frame_count);
    }

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return new_buf_stop;
}


void Envgen_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Envgen_vstate_render_voice;

    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    Time_env_state_init(&egen_state->env_state);

    return;
}


