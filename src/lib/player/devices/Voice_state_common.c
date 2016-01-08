

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


#include <player/devices/Voice_state_common.h>

#include <debug/assert.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <player/Audio_buffer.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Pitch_controls.h>
#include <player/Work_buffers.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define RAMP_RELEASE_TIME (200.0)


void Voice_state_common_handle_pitch(
        Voice_state* vstate,
        const Processor* proc,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(vstate != NULL);
    assert(proc != NULL);
    assert(wbs != NULL);
    assert(buf_start < buf_stop);

    Pitch_controls* pc = &vstate->pitch_controls;

    float* pitch_params = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    pitch_params[buf_start - 1] = pc->pitch;

    float* actual_pitches = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    actual_pitches[buf_start - 1] = vstate->actual_pitch;

    // Apply pitch slide
    if (Slider_in_progress(&pc->slider))
    {
        float new_pitch = pc->pitch;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_pitch = Slider_step(&pc->slider);
            pitch_params[i] = new_pitch;
        }
        pc->pitch = new_pitch;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitch_params[i] = pc->pitch;
    }

    // Adjust carried pitch
    if (pc->freq_mul != 1)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitch_params[i] *= pc->freq_mul;
    }

    // Initialise actual pitches
    memcpy(actual_pitches + buf_start,
            pitch_params + buf_start,
            sizeof(float) * (buf_stop - buf_start));

    // Apply vibrato
    if (LFO_active(&pc->vibrato))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            actual_pitches[i] *= LFO_step(&pc->vibrato);
    }

    // Apply arpeggio
    if (vstate->arpeggio)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            // Adjust actual pitch according to the current arpeggio state
            assert(!isnan(vstate->arpeggio_tones[0]));
            double diff = exp2(
                    (vstate->arpeggio_tones[vstate->arpeggio_note] -
                        vstate->arpeggio_ref) / 1200);
            actual_pitches[i] *= diff;

            // Update arpeggio state
            vstate->arpeggio_frames += 1;
            if (vstate->arpeggio_frames >= vstate->arpeggio_length)
            {
                vstate->arpeggio_frames -= vstate->arpeggio_length;
                ++vstate->arpeggio_note;
                if (vstate->arpeggio_note > KQT_ARPEGGIO_NOTES_MAX ||
                        isnan(vstate->arpeggio_tones[vstate->arpeggio_note]))
                    vstate->arpeggio_note = 0;
            }
        }
    }

    // Update actual pitch for next iteration
    vstate->actual_pitch = actual_pitches[buf_stop - 1];
    vstate->prev_actual_pitch = actual_pitches[buf_stop - 2];

    return;
}


#if 0
int32_t Voice_state_common_handle_force(
        Voice_state* vstate,
        const Au_state* au_state,
        const Processor* proc,
        const Work_buffers* wbs,
        uint32_t freq,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(vstate != NULL);
    assert(au_state != NULL);
    assert(proc != NULL);
    assert(wbs != NULL);

    float* actual_forces = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_FORCES);
    actual_forces[buf_start - 1] = vstate->actual_force;

    Force_controls* fc = &vstate->force_controls;

    int32_t new_buf_stop = buf_stop;

    // Apply force slide & global force
    if (Slider_in_progress(&fc->slider))
    {
        float new_force = fc->force;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
        {
            new_force = Slider_step(&fc->slider);
            actual_forces[i] = new_force * proc->au_params->global_force;
        }
        fc->force = new_force;
    }
    else
    {
        const float actual_force = fc->force * proc->au_params->global_force;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            actual_forces[i] = actual_force;
    }

    // Apply tremolo
    if (LFO_active(&fc->tremolo))
    {
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            actual_forces[i] *= LFO_step(&fc->tremolo);
    }

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

    return new_buf_stop;
}


int32_t Voice_state_common_ramp_release(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);

    const Processor* proc =
        (const Processor*)(((const Device_state*)proc_state)->device);

    const float ramp_start = vstate->ramp_release;
    int32_t process_stop = buf_stop;

    const float ramp_shift = RAMP_RELEASE_TIME / audio_rate;

    for (int32_t port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        const bool is_env_force_rel_used =
            Processor_is_voice_feature_enabled(proc, port, VOICE_FEATURE_FORCE) &&
            proc->au_params->env_force_rel_enabled;

        const bool do_ramp_release =
            !vstate->note_on &&
            Proc_state_is_voice_out_buffer_modified(proc_state, port) &&
            Processor_is_voice_feature_enabled(proc, port, VOICE_FEATURE_CUT) &&
            ((ramp_start > 0) ||
                (!is_env_force_rel_used && (au_state->sustain < 0.5)));

        if (do_ramp_release)
        {
            Work_buffer* buffer = Proc_state_get_voice_buffer_mut(
                    proc_state, DEVICE_PORT_TYPE_SEND, port);
            if (buffer != NULL)
            {
                float* buf = Work_buffer_get_contents_mut(buffer);

                float ramp = ramp_start;
                int32_t i = buf_start;

                for (i = buf_start; (i < buf_stop) && (ramp < 1); ++i)
                {
                    buf[i] *= 1 - ramp;
                    ramp += ramp_shift;
                }

                process_stop = min(i, process_stop);

                vstate->ramp_release = ramp;
            }
        }
    }

    return process_stop;
}
#endif


