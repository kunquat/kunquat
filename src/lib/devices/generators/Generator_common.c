

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
#include <devices/generators/Generator_common.h>
#include <Filter.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <player/Work_buffers.h>


#define RAMP_ATTACK_TIME (500.0)
#define RAMP_RELEASE_TIME (200.0)


void Generator_common_handle_pitch(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t nframes,
        int32_t offset)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    (void)gen;

    const Work_buffer* wb_pitch_params = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    const Work_buffer* wb_actual_pitches = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);

    float* pitch_params = Work_buffer_get_contents_mut(wb_pitch_params);
    pitch_params[offset - 1] = vstate->pitch;

    float* actual_pitches = Work_buffer_get_contents_mut(wb_actual_pitches);
    actual_pitches[offset - 1] = vstate->actual_pitch;

    // Apply pitch slide
    if (Slider_in_progress(&vstate->pitch_slider))
    {
        float new_pitch = vstate->pitch;
        for (int32_t i = offset; i < nframes; ++i)
        {
            new_pitch = Slider_step(&vstate->pitch_slider);
            pitch_params[i] = new_pitch;
        }
        vstate->pitch = new_pitch;
    }
    else
    {
        for (int32_t i = offset; i < nframes; ++i)
            pitch_params[i] = vstate->pitch;
    }

    // Initialise actual pitches
    memcpy(actual_pitches + offset,
            pitch_params + offset,
            sizeof(float) * (nframes - offset));

    // Apply vibrato
    if (LFO_active(&vstate->vibrato))
    {
        for (int32_t i = offset; i < nframes; ++i)
            actual_pitches[i] *= LFO_step(&vstate->vibrato);
    }

    // Apply arpeggio
    if (vstate->arpeggio)
    {
        for (int32_t i = offset; i < nframes; ++i)
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
    vstate->actual_pitch = actual_pitches[nframes - 1];
    vstate->prev_actual_pitch = actual_pitches[nframes - 2];

    return;
}


int32_t Generator_common_handle_force(
        const Generator* gen,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t freq,
        int32_t nframes,
        int32_t offset)
{
    assert(gen != NULL);
    assert(ins_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);

    const Work_buffer* wb_actual_forces = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_FORCES);

    float* actual_forces = Work_buffer_get_contents_mut(wb_actual_forces);
    actual_forces[offset - 1] = vstate->actual_force;

    int32_t buf_stop = nframes;

    // Apply force slide & global force
    if (Slider_in_progress(&vstate->force_slider))
    {
        float new_force = vstate->force;
        for (int32_t i = offset; i < buf_stop; ++i)
        {
            new_force = Slider_step(&vstate->force_slider);
            actual_forces[i] = new_force * gen->ins_params->global_force;
        }
        vstate->force = new_force;
    }
    else
    {
        const float actual_force = vstate->force * gen->ins_params->global_force;
        for (int32_t i = offset; i < buf_stop; ++i)
            actual_forces[i] = actual_force;
    }

    // Apply tremolo
    if (LFO_active(&vstate->tremolo))
    {
        for (int32_t i = offset; i < buf_stop; ++i)
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
                offset,
                buf_stop,
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
                buf_stop = env_force_stop;
            }
            else
            {
                // Fill the rest of the envelope buffer with the last value
                for (int32_t i = env_force_stop; i < buf_stop; ++i)
                    time_env[i] = last_value;
            }
        }

        for (int32_t i = offset; i < buf_stop; ++i)
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
                offset,
                buf_stop,
                freq);

        if (vstate->force_rel_env_state.is_finished)
            buf_stop = env_force_rel_stop;

        const Work_buffer* wb_time_env = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_TIME_ENV);
        float* time_env = Work_buffer_get_contents_mut(wb_time_env);

        for (int32_t i = offset; i < buf_stop; ++i)
            actual_forces[i] *= time_env[i];
    }

    // Update actual force for next iteration
    if (buf_stop < nframes)
        vstate->actual_force = 0;
    else if (buf_stop > offset)
        vstate->actual_force = actual_forces[buf_stop - 1];

    return buf_stop;
}


void Generator_common_handle_filter(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t nframes,
        int32_t offset)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);

    const Work_buffer* wb_actual_forces = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_FORCES);

    const float* actual_forces = Work_buffer_get_contents(wb_actual_forces);

    const Work_buffer* wb_actual_lowpasses = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_ACTUAL_LOWPASSES);

    float* actual_lowpasses = Work_buffer_get_contents_mut(wb_actual_lowpasses);

    // Apply lowpass slide
    if (Slider_in_progress(&vstate->lowpass_slider))
    {
        float new_lowpass = vstate->lowpass;
        for (int32_t i = offset; i < nframes; ++i)
        {
            new_lowpass = Slider_step(&vstate->lowpass_slider);
            actual_lowpasses[i] = new_lowpass;
        }
        vstate->lowpass = new_lowpass;
    }
    else
    {
        const float lowpass = vstate->lowpass;
        for (int32_t i = offset; i < nframes; ++i)
            actual_lowpasses[i] = lowpass;
    }

    // Apply autowah
    if (LFO_active(&vstate->autowah))
    {
        for (int32_t i = offset; i < nframes; ++i)
            actual_lowpasses[i] *= LFO_step(&vstate->autowah);
    }

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Work_buffer_get_contents_mut(wb_audio_l),
        Work_buffer_get_contents_mut(wb_audio_r),
    };

    for (int32_t i = offset; i < nframes; ++i)
    {
        vstate->actual_lowpass = actual_lowpasses[i];

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

        if (!vstate->lowpass_update &&
                vstate->lowpass_xfade_pos >= 1 &&
                (vstate->actual_lowpass < vstate->effective_lowpass * 0.98566319864018759 ||
                 vstate->actual_lowpass > vstate->effective_lowpass * 1.0145453349375237 ||
                 vstate->lowpass_resonance != vstate->effective_resonance))
        {
            vstate->lowpass_update = true;
            vstate->lowpass_xfade_state_used = vstate->lowpass_state_used;

            // TODO: figure out how to indicate start of note properly
            if ((vstate->pos > 0) || (i > offset))
                vstate->lowpass_xfade_pos = 0;
            else
                vstate->lowpass_xfade_pos = 1;

            vstate->lowpass_xfade_update = 200.0 / freq; // FIXME: / freq

            if (vstate->actual_lowpass < freq / 2)
            {
                int new_state = 1 - abs(vstate->lowpass_state_used);
                double lowpass = max(vstate->actual_lowpass, 1);
                two_pole_filter_create(lowpass / freq,
                        vstate->lowpass_resonance,
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

            vstate->effective_lowpass = vstate->actual_lowpass;
            vstate->effective_resonance = vstate->lowpass_resonance;
            vstate->lowpass_update = false;
        }

        if (vstate->lowpass_state_used > -1 || vstate->lowpass_xfade_state_used > -1)
        {
            assert(vstate->lowpass_state_used != vstate->lowpass_xfade_state_used);
            double result[KQT_BUFFERS_MAX] = { 0 };

            if (vstate->lowpass_state_used > -1)
            {
                Filter_state* fst =
                        &vstate->lowpass_state[vstate->lowpass_state_used];

                for (int a = 0; a < ab_count; ++a)
                {
                    result[a] = nq_zero_filter(
                            FILTER_ORDER,
                            fst->history1[a],
                            abufs[a][i]);
                    result[a] = iir_filter_strict_cascade(
                            FILTER_ORDER,
                            fst->coeffs,
                            fst->history2[a],
                            result[a]);
                    result[a] *= fst->mul;
                }
            }
            else
            {
                for (int a = 0; a < ab_count; ++a)
                    result[a] = abufs[a][i];
            }

            double vol = vstate->lowpass_xfade_pos;
            if (vol > 1)
                vol = 1;

            for (int a = 0; a < ab_count; ++a)
                result[a] *= vol;

            if (vstate->lowpass_xfade_pos < 1)
            {
                double fade_result[KQT_BUFFERS_MAX] = { 0 };

                if (vstate->lowpass_xfade_state_used > -1)
                {
                    Filter_state* fst =
                            &vstate->lowpass_state[vstate->lowpass_xfade_state_used];
                    for (int a = 0; a < ab_count; ++a)
                    {
                        fade_result[a] = nq_zero_filter(
                                FILTER_ORDER,
                                fst->history1[a],
                                abufs[a][i]);
                        fade_result[a] = iir_filter_strict_cascade(
                                FILTER_ORDER,
                                fst->coeffs,
                                fst->history2[a],
                                fade_result[a]);
                        fade_result[a] *= fst->mul;
                    }
                }
                else
                {
                    for (int a = 0; a < ab_count; ++a)
                        fade_result[a] = abufs[a][i];
                }

                double vol = 1 - vstate->lowpass_xfade_pos;
                if (vol > 0)
                {
                    for (int a = 0; a < ab_count; ++a)
                        result[a] += fade_result[a] * vol;
                }

                vstate->lowpass_xfade_pos += vstate->lowpass_xfade_update;
            }

            for (int a = 0; a < ab_count; ++a)
                abufs[a][i] = result[a];
        }
    }

    return;
}


#if 0
void Generator_common_handle_filter(
        const Generator* gen,
        Voice_state* vstate,
        double frames[],
        int frame_count,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);
    assert(freq > 0);

    if (Slider_in_progress(&vstate->lowpass_slider))
        vstate->lowpass = Slider_step(&vstate->lowpass_slider);

    vstate->actual_lowpass = vstate->lowpass;

    if (LFO_active(&vstate->autowah))
        vstate->actual_lowpass *= LFO_step(&vstate->autowah);

    if (gen->ins_params->env_force_filter_enabled &&
            vstate->lowpass_xfade_pos >= 1)
    {
        double force = vstate->actual_force;
        if (force > 1)
            force = 1;

        double factor = Envelope_get_value(
                gen->ins_params->env_force_filter,
                force);
        assert(isfinite(factor));
        vstate->actual_lowpass = min(vstate->actual_lowpass, 16384) * factor;
    }

    if (!vstate->lowpass_update &&
            vstate->lowpass_xfade_pos >= 1 &&
            (vstate->actual_lowpass < vstate->effective_lowpass * 0.98566319864018759 ||
             vstate->actual_lowpass > vstate->effective_lowpass * 1.0145453349375237 ||
             vstate->lowpass_resonance != vstate->effective_resonance))
    {
        vstate->lowpass_update = true;
        vstate->lowpass_xfade_state_used = vstate->lowpass_state_used;

        if (vstate->pos > 0)
            vstate->lowpass_xfade_pos = 0;
        else
            vstate->lowpass_xfade_pos = 1;

        vstate->lowpass_xfade_update = 200.0 / freq; // FIXME: / freq

        if (vstate->actual_lowpass < freq / 2)
        {
            int new_state = 1 - abs(vstate->lowpass_state_used);
            double lowpass = max(vstate->actual_lowpass, 1);
            two_pole_filter_create(lowpass / freq,
                    vstate->lowpass_resonance,
                    0,
                    vstate->lowpass_state[new_state].coeffs,
                    &vstate->lowpass_state[new_state].mul);
            for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
            {
                for (int k = 0; k < FILTER_ORDER; ++k)
                {
                    vstate->lowpass_state[new_state].history1[i][k] = 0;
                    vstate->lowpass_state[new_state].history2[i][k] = 0;
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

        vstate->effective_lowpass = vstate->actual_lowpass;
        vstate->effective_resonance = vstate->lowpass_resonance;
        vstate->lowpass_update = false;
    }

    if (vstate->lowpass_state_used > -1 || vstate->lowpass_xfade_state_used > -1)
    {
        assert(vstate->lowpass_state_used != vstate->lowpass_xfade_state_used);
        double result[KQT_BUFFERS_MAX] = { 0 };

        if (vstate->lowpass_state_used > -1)
        {
            Filter_state* fst =
                    &vstate->lowpass_state[vstate->lowpass_state_used];

            for (int i = 0; i < frame_count; ++i)
            {
                result[i] = nq_zero_filter(
                        FILTER_ORDER,
                        fst->history1[i],
                        frames[i]);
                result[i] = iir_filter_strict_cascade(
                        FILTER_ORDER,
                        fst->coeffs,
                        fst->history2[i],
                        result[i]);
                result[i] *= fst->mul;
            }
        }
        else
        {
            for (int i = 0; i < frame_count; ++i)
                result[i] = frames[i];
        }

        double vol = vstate->lowpass_xfade_pos;
        if (vol > 1)
            vol = 1;

        for (int i = 0; i < frame_count; ++i)
            result[i] *= vol;

        if (vstate->lowpass_xfade_pos < 1)
        {
            double fade_result[KQT_BUFFERS_MAX] = { 0 };

            if (vstate->lowpass_xfade_state_used > -1)
            {
                Filter_state* fst =
                        &vstate->lowpass_state[vstate->lowpass_xfade_state_used];
                for (int i = 0; i < frame_count; ++i)
                {
                    fade_result[i] = nq_zero_filter(
                            FILTER_ORDER,
                            fst->history1[i],
                            frames[i]);
                    fade_result[i] = iir_filter_strict_cascade(
                            FILTER_ORDER,
                            fst->coeffs,
                            fst->history2[i],
                            fade_result[i]);
                    fade_result[i] *= fst->mul;
                }
            }
            else
            {
                for (int i = 0; i < frame_count; ++i)
                    fade_result[i] = frames[i];
            }

            double vol = 1 - vstate->lowpass_xfade_pos;
            if (vol > 0)
            {
                for (int i = 0; i < frame_count; ++i)
                    result[i] += fade_result[i] * vol;
            }

            vstate->lowpass_xfade_pos += vstate->lowpass_xfade_update;
        }

        for (int i = 0; i < frame_count; ++i)
            frames[i] = result[i];
    }

    return;
}
#endif


void Generator_common_ramp_attack(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t nframes,
        int32_t offset)
{
    assert(gen != NULL);
    assert(vstate != NULL);

    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Work_buffer_get_contents_mut(wb_audio_l),
        Work_buffer_get_contents_mut(wb_audio_r),
    };

    const float start_ramp_attack = vstate->ramp_attack;
    const float inc = RAMP_ATTACK_TIME / freq;

    for (int ch = 0; ch < ab_count; ++ch)
    {
        float ramp_attack = start_ramp_attack;

        for (int32_t i = offset; (i < nframes) && (ramp_attack < 1); ++i)
        {
            abufs[ch][i] *= ramp_attack;
            ramp_attack += inc;
        }

        vstate->ramp_attack = ramp_attack;
    }

    return;
}


#if 0
void Generator_common_ramp_attack(
        const Generator* gen,
        Voice_state* vstate,
        double frames[],
        int frame_count,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);
    assert(freq > 0);
    (void)gen;

    if (vstate->ramp_attack < 1)
    {
        for (int i = 0; i < frame_count; ++i)
            frames[i] *= vstate->ramp_attack;

        vstate->ramp_attack += RAMP_ATTACK_TIME / freq;
    }

    return;
}
#endif


int32_t Generator_common_ramp_release(
        const Generator* gen,
        const Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t nframes,
        int32_t offset)
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
        const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_AUDIO_L);
        const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_AUDIO_R);

        float* abufs[KQT_BUFFERS_MAX] =
        {
            Work_buffer_get_contents_mut(wb_audio_l),
            Work_buffer_get_contents_mut(wb_audio_r),
        };

        const float ramp_shift = RAMP_RELEASE_TIME / freq;
        const float ramp_start = vstate->ramp_release;
        float ramp = ramp_start;
        int32_t i = offset;

        for (int ch = 0; ch < ab_count; ++ch)
        {
            ramp = ramp_start;
            for (i = offset; (i < nframes) && (ramp < 1); ++i)
            {
                if (ramp < 1)
                    abufs[ch][i] *= 1 - ramp;

                ramp += ramp_shift;
            }
        }

        vstate->ramp_release = ramp;

        return i;
    }

    return nframes;
}


void Generator_common_handle_panning(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t nframes,
        int32_t offset)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);

    const Work_buffer* wb_pitch_params = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_L);
    const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
            wbs, WORK_BUFFER_AUDIO_R);

    const float* pitch_params = Work_buffer_get_contents(wb_pitch_params);
    float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
    float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

    for (int32_t i = offset; i < nframes; ++i)
    {
        if (Slider_in_progress(&vstate->panning_slider))
            vstate->panning = Slider_step(&vstate->panning_slider);

        vstate->actual_panning = vstate->panning;

        if (gen->ins_params->env_pitch_pan_enabled)
        {
            Envelope* env = gen->ins_params->env_pitch_pan;
            double cents = log2(pitch_params[i] / 440) * 1200;
            if (cents < -6000)
                cents = -6000;
            else if (cents > 6000)
                cents = 6000;

            double pan = Envelope_get_value(env, cents);
            assert(isfinite(pan));
            double separation = 1 - fabs(vstate->actual_panning);
            vstate->actual_panning += pan * separation;

            if (vstate->actual_panning < -1)
                vstate->actual_panning = -1;
            else if (vstate->actual_panning > 1)
                vstate->actual_panning = 1;
        }

        audio_l[i] *= 1 - vstate->actual_panning;
        audio_r[i] *= 1 + vstate->actual_panning;
    }

    return;
}


#if 0
void Generator_common_handle_panning(
        const Generator* gen,
        Voice_state* vstate,
        double frames[],
        int frame_count)
{
    assert(gen != NULL);
    assert(vstate != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);

    if ((frame_count) >= 2)
    {
        if (Slider_in_progress(&vstate->panning_slider))
            vstate->panning = Slider_step(&vstate->panning_slider);

        vstate->actual_panning = vstate->panning;

        if (gen->ins_params->env_pitch_pan_enabled)
        {
            Envelope* env = gen->ins_params->env_pitch_pan;
            double cents = log2(vstate->pitch / 440) * 1200;
            if (cents < -6000)
                cents = -6000;
            else if (cents > 6000)
                cents = 6000;

            double pan = Envelope_get_value(env, cents);
            assert(isfinite(pan));
            double separation = 1 - fabs(vstate->actual_panning);
            vstate->actual_panning += pan * separation;

            if (vstate->actual_panning < -1)
                vstate->actual_panning = -1;
            else if (vstate->actual_panning > 1)
                vstate->actual_panning = 1;
        }

        frames[0] *= 1 - vstate->actual_panning;
        frames[1] *= 1 + vstate->actual_panning;
    }

    return;
}
#endif


