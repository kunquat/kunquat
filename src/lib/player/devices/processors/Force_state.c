

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
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
#include <init/devices/processors/Proc_force.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <player/Force_controls.h>
#include <player/Time_env_state.h>

#include <stdio.h>
#include <stdlib.h>


typedef struct Force_vstate
{
    Voice_state parent;
    double fixed_adjust;
} Force_vstate;


size_t Force_vstate_get_size(void)
{
    return sizeof(Force_vstate);
}


static int32_t Force_vstate_render_voice(
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

    // Get output
    float* out_buf =
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (out_buf == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    Force_vstate* fvstate = (Force_vstate*)vstate;
    //const Proc_force* force = (const Proc_force*)proc_state->parent.device->dimpl;

    Force_controls* fc = &vstate->force_controls;

    int32_t new_buf_stop = buf_stop;

    // Apply force slide & fixed adjust
    {
        const double fixed_scale = dB_to_scale(fvstate->fixed_adjust);
        if (Slider_in_progress(&fc->slider))
        {
            float new_force = fc->force;
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
            {
                new_force = Slider_step(&fc->slider);
                out_buf[i] = new_force * fixed_scale;
            }
            fc->force = new_force;
        }
        else
        {
            const float actual_force = fc->force * fixed_scale;
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buf[i] = actual_force;
        }
    }

    // Apply tremolo
    if (LFO_active(&fc->tremolo))
    {
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buf[i] *= LFO_step(&fc->tremolo);
    }

#if 0
    // Apply force envelope
    if (proc->au_params->env_force_enabled)
    {
        const Envelope* env = proc->au_params->env_force;

        const int32_t env_force_stop = Time_env_state_process(
                &vstate->force_env_state,
                env,
                proc->au_params->env_force_loop_enabled,
                proc->au_params->env_force_scale_amount,
                proc->au_params->env_force_center,
                0, // sustain
                0, 1, // range
                Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH),
                wbs,
                buf_start,
                new_buf_stop,
                freq);

        const Work_buffer* wb_time_env = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_TIME_ENV);
        float* time_env = Work_buffer_get_contents_mut(wb_time_env);

        // Check the end of envelope processing
        if (vstate->force_env_state.is_finished)
        {
            const double* last_node = Envelope_get_node(
                    env, Envelope_node_count(env) - 1);
            const double last_value = last_node[1];
            if (last_value == 0)
            {
                new_buf_stop = env_force_stop;
            }
            else
            {
                // Fill the rest of the envelope buffer with the last value
                for (int32_t i = env_force_stop; i < new_buf_stop; ++i)
                    time_env[i] = last_value;
            }
        }

        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            actual_forces[i] *= time_env[i];
    }

    // Apply force release envelope
    if (!vstate->note_on && proc->au_params->env_force_rel_enabled)
    {
        const Envelope* env = proc->au_params->env_force_rel;

        const int32_t env_force_rel_stop = Time_env_state_process(
                &vstate->force_rel_env_state,
                env,
                false, // no loop
                proc->au_params->env_force_rel_scale_amount,
                proc->au_params->env_force_rel_center,
                au_state->sustain,
                0, 1, // range
                Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH),
                wbs,
                buf_start,
                new_buf_stop,
                freq);

        if (vstate->force_rel_env_state.is_finished)
            new_buf_stop = env_force_rel_stop;

        const Work_buffer* wb_time_env = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_TIME_ENV);
        float* time_env = Work_buffer_get_contents_mut(wb_time_env);

        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            actual_forces[i] *= time_env[i];
    }

    // Update actual force for next iteration
    if (new_buf_stop < buf_stop)
        vstate->actual_force = 0;
    else if (new_buf_stop > buf_start)
        vstate->actual_force = actual_forces[new_buf_stop - 1];
#endif

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Force_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Force_vstate_render_voice;

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

    return;
}


