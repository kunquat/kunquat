

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <Filter.h>
#include <Generator_common.h>
#include <Generator.h>
#include <Voice_state.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <xassert.h>


#define RAMP_ATTACK_TIME (500.0)
#define RAMP_RELEASE_TIME (200.0)


void Generator_common_check_relative_lengths(Generator* gen,
                                             Voice_state* state,
                                             uint32_t freq,
                                             double tempo)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));
    (void)gen;
    if (state->freq != freq || state->tempo != tempo)
    {
        Slider_set_mix_rate(&state->pitch_slider, freq);
        Slider_set_tempo(&state->pitch_slider, tempo);
        LFO_set_mix_rate(&state->vibrato, freq);
        LFO_set_tempo(&state->vibrato, tempo);
        if (state->arpeggio)
        {
            state->arpeggio_length *= (double)freq / state->freq;
            state->arpeggio_length *= state->tempo / tempo;
            state->arpeggio_frames *= (double)freq / state->freq;
            state->arpeggio_frames *= state->tempo / tempo;
        }
        
        Slider_set_mix_rate(&state->force_slider, freq);
        Slider_set_tempo(&state->force_slider, tempo);
        LFO_set_mix_rate(&state->tremolo, freq);
        LFO_set_tempo(&state->tremolo, tempo);

        Slider_set_mix_rate(&state->panning_slider, freq);
        Slider_set_tempo(&state->panning_slider, tempo);

        Slider_set_mix_rate(&state->lowpass_slider, freq);
        Slider_set_tempo(&state->lowpass_slider, tempo);
        LFO_set_mix_rate(&state->autowah, freq);
        LFO_set_tempo(&state->autowah, tempo);

        state->freq = freq;
        state->tempo = tempo;
    }
    return;
}


void Generator_common_handle_pitch(Generator* gen,
                                   Voice_state* state)
{
    assert(gen != NULL);
    assert(state != NULL);
    state->prev_pitch = state->pitch;
    if (Slider_in_progress(&state->pitch_slider))
    {
        state->pitch = Slider_step(&state->pitch_slider);
    }
    state->prev_actual_pitch = state->actual_pitch;
    state->actual_pitch = state->pitch;
    if (gen->conf->pitch_lock_enabled)
    {
        state->pitch = state->actual_pitch = gen->conf->pitch_lock_freq;
        // TODO: The following alternative would enable a useful mode where
        //       the actual pitch is locked but other pitch-dependent mappings
        //       follow the original pitch.
//        state->actual_pitch = gen->conf->pitch_lock_freq;
    }
    else
    {
        if (LFO_active(&state->vibrato))
        {
            state->actual_pitch *= LFO_step(&state->vibrato);
        }
        if (state->arpeggio)
        {
            if (state->arpeggio_note > 0)
            {
                state->actual_pitch *= state->arpeggio_factors[
                                       state->arpeggio_note - 1];
            }
            state->arpeggio_frames += 1;
            if (state->arpeggio_frames >= state->arpeggio_length)
            {
                state->arpeggio_frames -= state->arpeggio_length;
                ++state->arpeggio_note;
                if (state->arpeggio_note > KQT_ARPEGGIO_NOTES_MAX
                        || state->arpeggio_factors[
                                state->arpeggio_note - 1] <= 0)
                {
                    state->arpeggio_note = 0;
                }
            }
        }
    }
    return;
}


void Generator_common_handle_force(Generator* gen,
                                   Voice_state* state,
                                   double frames[],
                                   int frame_count,
                                   uint32_t freq)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);
    if (Slider_in_progress(&state->force_slider))
    {
        state->force = Slider_step(&state->force_slider);
    }
    state->actual_force = state->force;
    if (LFO_active(&state->tremolo))
    {
        state->actual_force *= LFO_step(&state->tremolo);
    }
    if (gen->ins_params->env_force_enabled)
    {
        Envelope* env = gen->ins_params->env_force;

        int loop_start_index = Envelope_get_mark(env, 0);
        int loop_end_index = Envelope_get_mark(env, 1);
        double* loop_start = loop_start_index == -1 ? NULL :
                             Envelope_get_node(env, loop_start_index);
        double* loop_end = loop_end_index == -1 ? NULL :
                           Envelope_get_node(env, loop_end_index);
        if (gen->ins_params->env_force_scale_amount != 0 &&
                state->actual_pitch != state->prev_actual_pitch)
        {
            state->fe_scale = pow(state->actual_pitch /
                                      gen->ins_params->env_force_center,
                                  gen->ins_params->env_force_scale_amount);
        }

        double* next_node = Envelope_get_node(env, state->fe_next_node);
        double scale = NAN;
        if (next_node == NULL)
        {
            assert(loop_start == NULL);
            assert(loop_end == NULL);
            double* last_node = Envelope_get_node(env,
                                        Envelope_node_count(env) - 1);
            scale = last_node[1];
        }
        else if (state->fe_pos >= next_node[0])
        {
            ++state->fe_next_node;
            if (loop_end_index >= 0 && loop_end_index < state->fe_next_node)
            {
                assert(loop_start_index >= 0);
                state->fe_next_node = loop_start_index;
            }
            scale = Envelope_get_value(env, state->fe_pos);
            assert(isfinite(scale));
            double next_scale = Envelope_get_value(env, state->fe_pos +
                                                        1.0 / freq);
            state->fe_value = scale;
            state->fe_update = next_scale - scale;
        }
        else
        {
            assert(isfinite(state->fe_update));
            state->fe_value += state->fe_update * state->fe_scale;
            scale = state->fe_value;
            if (scale < 0)
            {
                scale = 0;
            }
        }
//        double scale = Envelope_get_value(env, state->fe_pos);
        assert(isfinite(scale));
        state->actual_force *= scale;
        double new_pos = state->fe_pos + state->fe_scale / freq;
        if (loop_start != NULL && loop_end != NULL)
        {
            if (new_pos > loop_end[0])
            {
                double loop_len = loop_end[0] - loop_start[0];
                assert(loop_len >= 0);
                if (loop_len == 0)
                {
                    new_pos = loop_end[0];
                }
                else
                {
                    double exceed = new_pos - loop_end[0];
                    double offset = fmod(exceed, loop_len);
                    new_pos = loop_start[0] + offset;
                    assert(new_pos >= loop_start[0]);
                    assert(new_pos <= loop_end[0]);
                    state->fe_next_node = loop_start_index;
                }
            }
        }
        else
        {
            double* last = Envelope_get_node(env,
                                             Envelope_node_count(env) - 1);
            if (new_pos > last[0])
            {
                new_pos = last[0];
                if (state->fe_pos > last[0] && last[1] == 0)
                {
                    state->active = false;
                    for (int i = 0; i < frame_count; ++i)
                    {
                        frames[i] = 0;
                    }
                    return;
                }
            }
        }
        state->fe_pos = new_pos;
    }
    if (!state->note_on)
    {
        if (gen->ins_params->env_force_rel_enabled)
        {
            if (gen->ins_params->env_force_rel_scale_amount != 0 &&
                    (state->actual_pitch != state->prev_actual_pitch ||
                     isnan(state->rel_fe_scale)))
            {
                state->rel_fe_scale = pow(state->actual_pitch /
                                          gen->ins_params->env_force_rel_center,
                                      gen->ins_params->env_force_rel_scale_amount);
            }
            else if (isnan(state->rel_fe_scale))
            {
                state->rel_fe_scale = 1;
            }
            Envelope* env = gen->ins_params->env_force_rel;
            double* next_node = Envelope_get_node(env, state->rel_fe_next_node);
            assert(next_node != NULL);
            double scale = NAN;
            if (state->rel_fe_pos >= next_node[0])
            {
                ++state->rel_fe_next_node;
                scale = Envelope_get_value(env, state->rel_fe_pos);
                if (!isfinite(scale))
                {
                    state->active = false;
                    for (int i = 0; i < frame_count; ++i)
                    {
                        frames[i] = 0;
                    }
                    return;
                }
                double next_scale = Envelope_get_value(env, state->rel_fe_pos +
                                                            1.0 / freq);
                state->rel_fe_value = scale;
                state->rel_fe_update = next_scale - scale;
            }
            else
            {
                assert(isfinite(state->rel_fe_update));
                state->rel_fe_value += state->rel_fe_update *
                                       state->rel_fe_scale * (1.0 - *state->pedal);
                scale = state->rel_fe_value;
                if (scale < 0)
                {
                    scale = 0;
                }
            }
#if 0
            double scale = Envelope_get_value(gen->ins_params->env_force_rel,
                                              state->rel_fe_pos);
            if (!isfinite(scale))
            {
                state->active = false;
                for (int i = 0; i < frame_count; ++i)
                {
                    frames[i] = 0;
                }
                return;
            }
#endif
            state->rel_fe_pos += state->rel_fe_scale * (1.0 - *state->pedal) / freq;
            state->actual_force *= scale;
        }
        else if (*state->pedal < 0.5)
        {
            if (state->ramp_release < 1)
            {
                for (int i = 0; i < frame_count; ++i)
                {
                    frames[i] *= 1 - state->ramp_release;
                }
            }
            else
            {
                state->active = false;
                for (int i = 0; i < frame_count; ++i)
                {
                    frames[i] = 0;
                }
                return;
            }
            state->ramp_release += RAMP_RELEASE_TIME / freq;
        }
    }
    for (int i = 0; i < frame_count; ++i)
    {
        frames[i] *= state->actual_force;
    }
    return;
}


void Generator_common_handle_filter(Generator* gen,
                                    Voice_state* state,
                                    double frames[],
                                    int frame_count,
                                    uint32_t freq)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);
    assert(freq > 0);
    if (Slider_in_progress(&state->lowpass_slider))
    {
        state->lowpass = Slider_step(&state->lowpass_slider);
    }
    state->actual_lowpass = state->lowpass;
    if (LFO_active(&state->autowah))
    {
        state->actual_lowpass *= LFO_step(&state->autowah);
    }
    if (gen->ins_params->env_force_filter_enabled &&
            state->lowpass_xfade_pos >= 1)
    {
        double force = state->actual_force;
        if (force > 1)
        {
            force = 1;
        }
        double factor = Envelope_get_value(gen->ins_params->env_force_filter,
                                           force);
        assert(isfinite(factor));
        state->actual_lowpass = MIN(state->actual_lowpass, 16384) * factor;
    }

    if (!state->lowpass_update &&
            state->lowpass_xfade_pos >= 1 &&
            (state->actual_lowpass < state->effective_lowpass * 0.98566319864018759 ||
             state->actual_lowpass > state->effective_lowpass * 1.0145453349375237 ||
             state->lowpass_resonance != state->effective_resonance))
    {
        state->lowpass_update = true;
        state->lowpass_xfade_state_used = state->lowpass_state_used;
        if (state->pos > 0)
        {
            state->lowpass_xfade_pos = 0;
        }
        else
        {
            state->lowpass_xfade_pos = 1;
        }
        state->lowpass_xfade_update = 200.0 / freq; // FIXME: / freq
        if (state->actual_lowpass < freq / 2)
        {
            int new_state = 1 - abs(state->lowpass_state_used);
            two_pole_lowpass_filter_create(state->actual_lowpass / freq,
                    state->lowpass_resonance,
                    state->lowpass_state[new_state].coeffs,
                    &state->lowpass_state[new_state].a0);
            for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
            {
                for (int k = 0; k < FILTER_ORDER; ++k)
                {
                    state->lowpass_state[new_state].history1[i][k] = 0;
                    state->lowpass_state[new_state].history2[i][k] = 0;
                }
            }
            state->lowpass_state_used = new_state;
//            fprintf(stderr, "created filter with cutoff %f\n", state->actual_filter);
        }
        else
        {
            if (state->lowpass_state_used == -1)
            {
                state->lowpass_xfade_pos = 1;
            }
            state->lowpass_state_used = -1;
        }
        state->effective_lowpass = state->actual_lowpass;
        state->effective_resonance = state->lowpass_resonance;
        state->lowpass_update = false;
    }

    if (state->lowpass_state_used > -1 || state->lowpass_xfade_state_used > -1)
    {
        assert(state->lowpass_state_used != state->lowpass_xfade_state_used);
        double result[KQT_BUFFERS_MAX] = { 0 };
        if (state->lowpass_state_used > -1)
        {
            Filter_state* fst =
                    &state->lowpass_state[state->lowpass_state_used];
            for (int i = 0; i < frame_count; ++i)
            {
                result[i] = nq_zero_filter(FILTER_ORDER,
                                           fst->history1[i],
                                           frames[i]);
                result[i] = iir_filter_strict_cascade(FILTER_ORDER,
                                                      fst->coeffs,
                                                      fst->history2[i],
                                                      result[i]);
                result[i] /= fst->a0;
            }
        }
        else
        {
            for (int i = 0; i < frame_count; ++i)
            {
                result[i] = frames[i];
            }
        }
        double vol = state->lowpass_xfade_pos;
        if (vol > 1)
        {
            vol = 1;
        }
        for (int i = 0; i < frame_count; ++i)
        {
            result[i] *= vol;
        }
        if (state->lowpass_xfade_pos < 1)
        {
            double fade_result[KQT_BUFFERS_MAX] = { 0 };
            if (state->lowpass_xfade_state_used > -1)
            {
                Filter_state* fst =
                        &state->lowpass_state[state->lowpass_xfade_state_used];
                for (int i = 0; i < frame_count; ++i)
                {
                    fade_result[i] = nq_zero_filter(FILTER_ORDER,
                                                    fst->history1[i],
                                                    frames[i]);
                    fade_result[i] = iir_filter_strict_cascade(FILTER_ORDER,
                                                               fst->coeffs,
                                                               fst->history2[i],
                                                               fade_result[i]);
                    fade_result[i] /= fst->a0;
                }
            }
            else
            {
                for (int i = 0; i < frame_count; ++i)
                {
                    fade_result[i] = frames[i];
                }
            }
            double vol = 1 - state->lowpass_xfade_pos;
            if (vol > 0)
            {
                for (int i = 0; i < frame_count; ++i)
                {
                    result[i] += fade_result[i] * vol;
                }
            }
            state->lowpass_xfade_pos += state->lowpass_xfade_update;
        }
        for (int i = 0; i < frame_count; ++i)
        {
            frames[i] = result[i];
        }
    }
    return;
}


void Generator_common_ramp_attack(Generator* gen,
                                  Voice_state* state,
                                  double frames[],
                                  int frame_count,
                                  uint32_t freq)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);
    assert(freq > 0);
    (void)gen;
    if (state->ramp_attack < 1)
    {
        for (int i = 0; i < frame_count; ++i)
        {
            frames[i] *= state->ramp_attack;
        }
        state->ramp_attack += RAMP_ATTACK_TIME / freq;
    }
    return;
}


void Generator_common_handle_panning(Generator* gen,
                                     Voice_state* state,
                                     double frames[],
                                     int frame_count)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(frames != NULL);
    assert(frame_count > 0);
    if ((frame_count) >= 2)
    {
        if (Slider_in_progress(&state->panning_slider))
        {
            state->panning = Slider_step(&state->panning_slider);
        }
        (state)->actual_panning = (state)->panning;
        if ((gen)->ins_params->env_pitch_pan_enabled)
        {
            Envelope* env = (gen)->ins_params->env_pitch_pan;
            double cents = log2((state)->pitch / 440) * 1200;
            if (cents < -6000)
            {
                cents = -6000;
            }
            else if (cents > 6000)
            {
                cents = 6000;
            }
            double pan = Envelope_get_value(env, cents);
            assert(isfinite(pan));
            double separation = 1 - fabs((state)->actual_panning);
            (state)->actual_panning += pan * separation;
            if ((state)->actual_panning < -1)
            {
                (state)->actual_panning = -1;
            }
            else if ((state)->actual_panning > 1)
            {
                (state)->actual_panning = 1;
            }
        }
        (frames)[0] *= 1 - (state)->actual_panning;
        (frames)[1] *= 1 + (state)->actual_panning;
    }
    return;
}


