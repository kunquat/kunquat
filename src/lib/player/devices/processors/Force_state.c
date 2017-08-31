

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Force_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_force.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Force_controls.h>
#include <player/Time_env_state.h>
#include <player/Work_buffers.h>

#include <stdio.h>
#include <stdlib.h>


typedef struct Force_vstate
{
    Voice_state parent;

    double fixed_adjust;
    double release_ramp_progress;

    Force_controls controls;
    Time_env_state env_state;
    Time_env_state release_env_state;
} Force_vstate;


int32_t Force_vstate_get_size(void)
{
    return sizeof(Force_vstate);
}


#define RAMP_RELEASE_SPEED 200.0


enum
{
    PORT_IN_ENV_STRETCH = 0,
    PORT_IN_ENV_REL_STRETCH,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_FORCE = 0,
    PORT_OUT_COUNT
};


static const int FORCE_WB_FIXED_ENV_STRETCH = WORK_BUFFER_IMPL_1;
static const int FORCE_WB_FIXED_ENV_REL_STRETCH = WORK_BUFFER_IMPL_2;


int32_t Force_vstate_render_voice(
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

    const Device_state* dstate = &proc_state->parent;

    // Get envelope time stretch inputs
    Work_buffer* stretch_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_ENV_STRETCH);
    if (stretch_wb == NULL)
    {
        stretch_wb = Work_buffers_get_buffer_mut(wbs, FORCE_WB_FIXED_ENV_STRETCH);
        float* stretches = Work_buffer_get_contents_mut(stretch_wb);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            stretches[i] = 0;
        Work_buffer_set_const_start(stretch_wb, buf_start);
    }
    else
    {
        Proc_clamp_pitch_values(stretch_wb, buf_start, buf_stop);
    }

    Work_buffer* rel_stretch_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_ENV_REL_STRETCH);
    if (rel_stretch_wb == NULL)
    {
        rel_stretch_wb =
            Work_buffers_get_buffer_mut(wbs, FORCE_WB_FIXED_ENV_REL_STRETCH);
        float* stretches = Work_buffer_get_contents_mut(rel_stretch_wb);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            stretches[i] = 0;
        Work_buffer_set_const_start(rel_stretch_wb, buf_start);
    }
    else
    {
        Proc_clamp_pitch_values(rel_stretch_wb, buf_start, buf_stop);
    }

    // Get output
    Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_FORCE);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return buf_start;
    }
    float* out_buf = Work_buffer_get_contents_mut(out_wb);

    Force_vstate* fvstate = (Force_vstate*)vstate;
    const Proc_force* force = (const Proc_force*)proc_state->parent.device->dimpl;

    Force_controls* fc = &fvstate->controls;
    Force_controls_set_audio_rate(fc, dstate->audio_rate);
    Force_controls_set_tempo(fc, tempo);

    int32_t const_start = buf_start;

    int32_t new_buf_stop = buf_stop;

    // Apply force slide & fixed adjust
    {
        const double fixed_adjust = fvstate->fixed_adjust;

        int32_t cur_pos = buf_start;
        while (cur_pos < buf_stop)
        {
            const int32_t estimated_steps =
                Slider_estimate_active_steps_left(&fc->slider);
            if (estimated_steps > 0)
            {
                int32_t slide_stop = buf_stop;
                if (estimated_steps < buf_stop - cur_pos)
                    slide_stop = cur_pos + estimated_steps;

                double new_force = fc->force;
                for (int32_t i = cur_pos; i < slide_stop; ++i)
                {
                    new_force = Slider_step(&fc->slider);
                    out_buf[i] = (float)(new_force + fixed_adjust);
                }
                fc->force = new_force;

                const_start = slide_stop;
                cur_pos = slide_stop;
            }
            else
            {
                const float actual_force = (float)(fc->force + fixed_adjust);
                for (int32_t i = cur_pos; i < buf_stop; ++i)
                    out_buf[i] = actual_force;

                cur_pos = buf_stop;
            }
        }
    }

    // Apply tremolo
    {
        int32_t cur_pos = buf_start;
        int32_t final_lfo_stop = buf_start;
        while (cur_pos < buf_stop)
        {
            const int32_t estimated_steps =
                LFO_estimate_active_steps_left(&fc->tremolo);
            if (estimated_steps > 0)
            {
                int32_t lfo_stop = buf_stop;
                if (estimated_steps < buf_stop - cur_pos)
                    lfo_stop = cur_pos + estimated_steps;

                for (int32_t i = cur_pos; i < lfo_stop; ++i)
                    out_buf[i] += (float)LFO_step(&fc->tremolo);

                final_lfo_stop = lfo_stop;
                cur_pos = lfo_stop;
            }
            else
            {
                final_lfo_stop = cur_pos;
                break;
            }
        }

        const_start = max(const_start, final_lfo_stop);
    }

    int32_t keep_alive_stop = buf_stop;

    // Apply force envelope
    if (force->is_force_env_enabled && (force->force_env != NULL))
    {
        const Envelope* env = force->force_env;

        const int32_t env_force_stop = Time_env_state_process(
                &fvstate->env_state,
                env,
                force->is_force_env_loop_enabled,
                0, // sustain
                0, 1, // range
                stretch_wb,
                Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV),
                buf_start,
                new_buf_stop,
                proc_state->parent.audio_rate);

        const_start = max(const_start, env_force_stop);

        Work_buffer* wb_time_env =
            Work_buffers_get_buffer_mut(wbs, WORK_BUFFER_TIME_ENV);
        float* time_env = Work_buffer_get_contents_mut(wb_time_env);

        // Convert envelope data to dB
        for (int32_t i = buf_start; i < env_force_stop; ++i)
            time_env[i] = (float)fast_scale_to_dB(time_env[i]);

        // Check the end of envelope processing
        if (fvstate->env_state.is_finished)
        {
            const double* last_node = Envelope_get_node(
                    env, Envelope_node_count(env) - 1);
            const double last_value = last_node[1];
            if (last_value == 0)
            {
                new_buf_stop = env_force_stop;

                for (int32_t i = new_buf_stop; i < buf_stop; ++i)
                    out_buf[i] = -INFINITY;

                keep_alive_stop = min(keep_alive_stop, new_buf_stop);
            }
            else
            {
                // Fill the rest of the envelope buffer with the last value
                const float last_value_dB = (float)scale_to_dB(last_value);
                for (int32_t i = env_force_stop; i < new_buf_stop; ++i)
                    time_env[i] = last_value_dB;
            }
        }

        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buf[i] += time_env[i];
    }

    if (!vstate->note_on)
    {
        const_start = buf_stop;

        if (force->is_force_release_env_enabled)
        {
            // Apply force release envelope
            const Envelope* env = force->force_release_env;
            rassert(env != NULL);

            const int32_t env_force_rel_stop = Time_env_state_process(
                    &fvstate->release_env_state,
                    env,
                    false, // no loop
                    au_state->sustain,
                    0, 1, // range
                    rel_stretch_wb,
                    Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV),
                    buf_start,
                    new_buf_stop,
                    proc_state->parent.audio_rate);

            if (fvstate->release_env_state.is_finished)
                new_buf_stop = env_force_rel_stop;

            Work_buffer* wb_time_env =
                Work_buffers_get_buffer_mut(wbs, WORK_BUFFER_TIME_ENV);
            float* time_env = Work_buffer_get_contents_mut(wb_time_env);

            // Convert envelope data to dB
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                time_env[i] = (float)fast_scale_to_dB(time_env[i]);

            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buf[i] += time_env[i];

            for (int32_t i = new_buf_stop; i < buf_stop; ++i)
                out_buf[i] = -INFINITY;

            if (fvstate->release_env_state.is_finished)
                keep_alive_stop = min(keep_alive_stop, new_buf_stop);
        }
        else if (force->is_release_ramping_enabled)
        {
            // Apply release ramping
            new_buf_stop = buf_stop;

            const float ramp_step =
                (float)(RAMP_RELEASE_SPEED / proc_state->parent.audio_rate);

            if (au_state->sustain < 0.5)
            {
                double progress = fvstate->release_ramp_progress;
                int32_t i = buf_start;

                for (i = buf_start; (i < buf_stop) && (progress < 1); ++i)
                {
                    out_buf[i] += (float)fast_scale_to_dB(1 - progress);
                    progress += ramp_step;
                }

                new_buf_stop = min(i, new_buf_stop);

                fvstate->release_ramp_progress = progress;
            }

            for (int32_t i = new_buf_stop; i < buf_stop; ++i)
                out_buf[i] = -INFINITY;

            if (fvstate->release_ramp_progress >= 1)
                keep_alive_stop = min(keep_alive_stop, new_buf_stop);
        }
        else
        {
            keep_alive_stop = buf_start;
        }
    }

    // Mark constant region of the buffer
    Work_buffer_set_const_start(out_wb, const_start);
    Work_buffer_set_final(out_wb, keep_alive_stop < buf_stop);

    Voice_state_set_keep_alive_stop(vstate, keep_alive_stop);

    return buf_stop;
}


void Force_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    const Proc_force* force = (const Proc_force*)proc_state->parent.device->dimpl;
    Force_vstate* fvstate = (Force_vstate*)vstate;

    // Set fixed force adjustment
    fvstate->fixed_adjust = force->global_force;

    if (force->force_var != 0)
    {
        double var_dB = Random_get_float_scale(vstate->rand_p) * force->force_var * 2.0;
        var_dB -= force->force_var;

        fvstate->fixed_adjust += var_dB;
    }

    fvstate->release_ramp_progress = 0.0;

    Force_controls_init(&fvstate->controls, proc_state->parent.audio_rate, 120);

    Time_env_state_init(&fvstate->env_state);
    Time_env_state_init(&fvstate->release_env_state);

    return;
}


Force_controls* Force_vstate_get_force_controls_mut(Voice_state* vstate)
{
    rassert(vstate != NULL);
    rassert(vstate->proc_type == Proc_type_force);

    Force_vstate* fvstate = (Force_vstate*)vstate;

    return &fvstate->controls;
}


