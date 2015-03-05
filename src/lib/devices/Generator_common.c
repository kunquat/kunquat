

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <debug/assert.h>
#include <devices/Generator.h>
#include <devices/Generator_common.h>
#include <Filter.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <player/Work_buffers.h>


#define RAMP_RELEASE_TIME (200.0)


void Generator_common_handle_pitch(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    (void)gen;

    float* pitch_params = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    pitch_params[buf_start - 1] = vstate->pitch;

    float* actual_pitches = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    actual_pitches[buf_start - 1] = vstate->actual_pitch;

    // Apply pitch slide
    if (Slider_in_progress(&vstate->pitch_slider))
    {
        float new_pitch = vstate->pitch;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_pitch = Slider_step(&vstate->pitch_slider);
            pitch_params[i] = new_pitch;
        }
        vstate->pitch = new_pitch;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitch_params[i] = vstate->pitch;
    }

    // Initialise actual pitches
    memcpy(actual_pitches + buf_start,
            pitch_params + buf_start,
            sizeof(float) * (buf_stop - buf_start));

    // Apply vibrato
    if (LFO_active(&vstate->vibrato))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            actual_pitches[i] *= LFO_step(&vstate->vibrato);
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


int32_t Generator_common_handle_force(
        const Generator* gen,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t freq,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gen != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);

    float* actual_forces = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_FORCES);
    actual_forces[buf_start - 1] = vstate->actual_force;

    int32_t new_buf_stop = buf_stop;

    // Apply force slide & global force
    if (Slider_in_progress(&vstate->force_slider))
    {
        float new_force = vstate->force;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
        {
            new_force = Slider_step(&vstate->force_slider);
            actual_forces[i] = new_force * gen->ins_params->global_force;
        }
        vstate->force = new_force;
    }
    else
    {
        const float actual_force = vstate->force * gen->ins_params->global_force;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            actual_forces[i] = actual_force;
    }

    // Apply tremolo
    if (LFO_active(&vstate->tremolo))
    {
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            actual_forces[i] *= LFO_step(&vstate->tremolo);
    }

    // Apply force envelope
    if (gen->ins_params->env_force_enabled)
    {
        const Envelope* env = gen->ins_params->env_force;

        const int32_t env_force_stop = Time_env_state_process(
                &vstate->force_env_state,
                env,
                gen->ins_params->env_force_scale_amount,
                gen->ins_params->env_force_center,
                0, // sustain
                0, 1, // range
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
    if (!vstate->note_on && gen->ins_params->env_force_rel_enabled)
    {
        const Envelope* env = gen->ins_params->env_force_rel;

        const int32_t env_force_rel_stop = Time_env_state_process(
                &vstate->force_rel_env_state,
                env,
                gen->ins_params->env_force_rel_scale_amount,
                gen->ins_params->env_force_rel_center,
                ins_state->sustain,
                0, 1, // range
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


void apply_filter_settings(
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        double xfade_start,
        double xfade_step,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(vstate != NULL);
    assert(wbs != NULL);

    if ((vstate->lowpass_state_used == -1) && (vstate->lowpass_xfade_state_used == -1))
        return;

    assert(vstate->lowpass_state_used != vstate->lowpass_xfade_state_used);

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L),
        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R),
    };

    // Get filter states used
    Filter_state* in_fst = (vstate->lowpass_state_used > -1) ?
        &vstate->lowpass_state[vstate->lowpass_state_used] : NULL;
    Filter_state* out_fst = (vstate->lowpass_xfade_state_used > -1) ?
        &vstate->lowpass_state[vstate->lowpass_xfade_state_used] : NULL;

    const double xfade_start_clamped = min(xfade_start, 1);

    for (int32_t ch = 0; ch < ab_count; ++ch)
    {
        double xfade = xfade_start_clamped;

        float* audio_buffer = abufs[ch];

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float input = audio_buffer[i];
            double result = input;

            // Apply primary filter
            if (in_fst != NULL)
            {
                result = nq_zero_filter(
                        FILTER_ORDER, in_fst->history1[ch], input);
                result = iir_filter_strict_cascade_even_order(
                        FILTER_ORDER, in_fst->coeffs, in_fst->history2[ch], result);
                result *= in_fst->mul;
            }

            // Apply secondary filter with crossfade
            if (xfade < 1)
            {
                result *= xfade;

                double fade_result = input;

                if (out_fst != NULL)
                {
                    fade_result = nq_zero_filter(
                            FILTER_ORDER, out_fst->history1[ch], input);
                    fade_result = iir_filter_strict_cascade_even_order(
                            FILTER_ORDER,
                            out_fst->coeffs,
                            out_fst->history2[ch],
                            fade_result);
                    fade_result *= out_fst->mul;
                }

                result += fade_result * (1 - xfade);

                xfade += xfade_step;
            }

            audio_buffer[i] = result;
        }
    }

    return;
}


void Generator_common_handle_filter(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert((ab_count == 1) || (ab_count == 2));

    const float* actual_forces = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_FORCES);

    float* actual_lowpasses = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_LOWPASSES);

    // Apply lowpass slide
    if (Slider_in_progress(&vstate->lowpass_slider))
    {
        float new_lowpass = vstate->lowpass;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_lowpass = Slider_step(&vstate->lowpass_slider);
            actual_lowpasses[i] = new_lowpass;
        }
        vstate->lowpass = new_lowpass;
    }
    else
    {
        const float lowpass = vstate->lowpass;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            actual_lowpasses[i] = lowpass;
    }

    // Apply autowah
    if (LFO_active(&vstate->autowah))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            actual_lowpasses[i] *= LFO_step(&vstate->autowah);
    }

    static const double max_true_lowpass_change = 1.0145453349375237; // 2^(1/48)
    static const double min_true_lowpass_change = 1.0 / max_true_lowpass_change;

    const double xfade_step = 200.0 / freq;
    vstate->lowpass_xfade_update = xfade_step;

    const double nyquist = (double)freq * 0.5;

    int32_t apply_filter_start = buf_start;
    int32_t apply_filter_stop = buf_stop;
    double xfade_start = vstate->lowpass_xfade_pos;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        vstate->actual_lowpass = actual_lowpasses[i];

        // Apply force->filter envelope
        if (gen->ins_params->env_force_filter_enabled &&
                vstate->lowpass_xfade_pos >= 1)
        {
            double force = actual_forces[i];
            if (force > 1)
                force = 1;

            double factor = Envelope_get_value(
                    gen->ins_params->env_force_filter,
                    force);
            assert(isfinite(factor));
            vstate->actual_lowpass = min(vstate->actual_lowpass, 16384) * factor;
        }

        // Initialise new filter settings if needed
        if (vstate->lowpass_xfade_pos >= 1 &&
                (vstate->actual_lowpass < vstate->true_lowpass * min_true_lowpass_change ||
                 vstate->actual_lowpass > vstate->true_lowpass * max_true_lowpass_change ||
                 vstate->lowpass_resonance != vstate->true_resonance))
        {
            // Apply previous filter settings to the signal
            apply_filter_stop = i;
            apply_filter_settings(
                    vstate,
                    wbs,
                    ab_count,
                    xfade_start,
                    xfade_step,
                    apply_filter_start,
                    apply_filter_stop);

            // Set up new range for next filter processing
            apply_filter_start = i;
            apply_filter_stop = buf_stop;

            vstate->lowpass_xfade_state_used = vstate->lowpass_state_used;

            // TODO: figure out how to indicate start of note properly
            if ((vstate->pos > 0) || (i > buf_start))
                vstate->lowpass_xfade_pos = 0;
            else
                vstate->lowpass_xfade_pos = 1;

            vstate->true_lowpass = vstate->actual_lowpass;
            vstate->true_resonance = vstate->lowpass_resonance;

            if (vstate->true_lowpass < nyquist)
            {
                int new_state = 1 - abs(vstate->lowpass_state_used);
                double lowpass = max(vstate->true_lowpass, 1);
                two_pole_filter_create(lowpass / freq,
                        vstate->true_resonance,
                        0,
                        vstate->lowpass_state[new_state].coeffs,
                        &vstate->lowpass_state[new_state].mul);
                for (int a = 0; a < KQT_BUFFERS_MAX; ++a)
                {
                    for (int k = 0; k < FILTER_ORDER; ++k)
                    {
                        vstate->lowpass_state[new_state].history1[a][k] = 0;
                        vstate->lowpass_state[new_state].history2[a][k] = 0;
                    }
                }
                vstate->lowpass_state_used = new_state;
    //            fprintf(stderr, "created filter with cutoff %f\n", vstate->actual_filter);
            }
            else
            {
                if (vstate->lowpass_state_used == -1)
                    vstate->lowpass_xfade_pos = 1;

                vstate->lowpass_state_used = -1;
            }

            xfade_start = vstate->lowpass_xfade_pos;
        }

        vstate->lowpass_xfade_pos += xfade_step;
    }

    // Apply previous filter settings to the remaining signal
    apply_filter_settings(
            vstate,
            wbs,
            ab_count,
            xfade_start,
            xfade_step,
            apply_filter_start,
            apply_filter_stop);

    return;
}


int32_t Generator_common_ramp_release(
        const Generator* gen,
        const Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);

    const bool do_ramp_release =
        !vstate->note_on &&
        ((vstate->ramp_release > 0) ||
            (!gen->ins_params->env_force_rel_enabled && (ins_state->sustain < 0.5)));

    if (do_ramp_release)
    {
        float* abufs[KQT_BUFFERS_MAX] =
        {
            Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L),
            Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R),
        };

        const float ramp_shift = RAMP_RELEASE_TIME / freq;
        const float ramp_start = vstate->ramp_release;
        float ramp = ramp_start;
        int32_t i = buf_start;

        for (int ch = 0; ch < ab_count; ++ch)
        {
            ramp = ramp_start;
            for (i = buf_start; (i < buf_stop) && (ramp < 1); ++i)
            {
                abufs[ch][i] *= 1 - ramp;
                ramp += ramp_shift;
            }
        }

        vstate->ramp_release = ramp;

        return i;
    }

    return buf_stop;
}


void Generator_common_handle_panning(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);

    const float* pitch_params = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    float* actual_pannings = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_PANNINGS);
    float* audio_l = Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_L);
    float* audio_r = Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_AUDIO_R);

    // Apply panning slide
    if (Slider_in_progress(&vstate->panning_slider))
    {
        float new_panning = vstate->panning;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_panning = Slider_step(&vstate->panning_slider);
            actual_pannings[i] = new_panning;
        }
        vstate->panning = new_panning;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            actual_pannings[i] = vstate->panning;
    }

    // Apply pitch->pan envelope
    if (gen->ins_params->env_pitch_pan_enabled)
    {
        const Envelope* env = gen->ins_params->env_pitch_pan;

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float pitch_param = pitch_params[i];

            float actual_panning = actual_pannings[i];

            if (pitch_param != vstate->pitch_pan_ref_param)
            {
                double cents = log2(pitch_params[i] / 440) * 1200;
                cents = clamp(cents, -6000, 6000);

                const float pan = Envelope_get_value(env, cents);
                assert(isfinite(pan));

                vstate->pitch_pan_ref_param = pitch_param;
                vstate->pitch_pan_value = pan;
            }

            double separation = 1 - fabs(actual_panning);
            actual_panning += vstate->pitch_pan_value * separation;
            actual_panning = clamp(actual_panning, -1, 1);

            actual_pannings[i] = actual_panning;
        }
    }

    // Apply final panning to the audio signal
    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_panning = actual_pannings[i];

        audio_l[i] *= 1 - actual_panning;
        audio_r[i] *= 1 + actual_panning;
    }

    return;
}


