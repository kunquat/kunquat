

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
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <Voice_state.h>
#include <Generator.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <math_common.h>


#define RAMP_ATTACK_TIME (500.0)
#define RAMP_RELEASE_TIME (200.0)


/**
 * Checks whether there's anything left of the note to be mixed.
 * This should be called at the beginning of the mixing function of the
 * Generator.
 */
#define Generator_common_check_active(gen, state, mixed)                     \
    if (true)                                                                \
    {                                                                        \
        if (!(state)->active || (!(state)->note_on &&                        \
                                 ((state)->pos == 0) &&                      \
                                 ((state)->pos_rem == 0) &&                  \
                                 !(gen)->ins_params->env_force_rel_enabled)) \
        {                                                                    \
            (state)->active = false;                                         \
            return (mixed);                                                  \
        }                                                                    \
    } else (void)0


/**
 * Adjusts Voice state according to tempo and/or mixing frequency changes.
 * This should be called before the mixing loop of the Generator.
 */
#define Generator_common_check_relative_lengths(gen, state, freq, tempo)          \
    if (true)                                                                     \
    {                                                                             \
        if ((state)->freq != (freq) || (state)->tempo != (tempo))                 \
        {                                                                         \
            if ((state)->pitch_slide != 0)                                        \
            {                                                                     \
                double slide_step = log2((state)->pitch_slide_update);            \
                slide_step *= (double)(state)->freq / (freq);                     \
                slide_step *= (tempo) / (state)->tempo;                           \
                (state)->pitch_slide_update = exp2(slide_step);                   \
                (state)->pitch_slide_frames *= (double)(freq) / (state)->freq;    \
                (state)->pitch_slide_frames *= (state)->tempo / (tempo);          \
            }                                                                     \
            if ((state)->vibrato_length > 0 && (state)->vibrato_depth > 0)        \
            {                                                                     \
                (state)->vibrato_phase *= (double)(freq) / (state)->freq;         \
                (state)->vibrato_phase *= (state)->tempo / (tempo);               \
            }                                                                     \
            (state)->vibrato_length *= (double)(freq) / (state)->freq;            \
            (state)->vibrato_length *= (state)->tempo / (tempo);                  \
            (state)->vibrato_update *= (double)(state)->freq / (freq);            \
            (state)->vibrato_update *= (tempo) / (state)->tempo;                  \
            (state)->vibrato_delay_update *= (double)(state)->freq / (freq);      \
            (state)->vibrato_delay_update *= (tempo) / (state)->tempo;            \
            if ((state)->arpeggio)                                                \
            {                                                                     \
                (state)->arpeggio_length *= (double)(freq) / (state)->freq;       \
                (state)->arpeggio_length *= (state)->tempo / (tempo);             \
                (state)->arpeggio_frames *= (double)(freq) / (state)->freq;       \
                (state)->arpeggio_frames *= (state)->tempo / (tempo);             \
            }                                                                     \
            if ((state)->force_slide != 0)                                        \
            {                                                                     \
                double update_dB = log2((state)->force_slide_update) * 6;         \
                update_dB *= (double)(state)->freq / (freq);                      \
                update_dB *= (tempo) / (state)->tempo;                            \
                (state)->force_slide_update = exp2(update_dB / 6);                \
                (state)->force_slide_frames *= (double)(freq) / (state)->freq;    \
                (state)->force_slide_frames *= (state)->tempo / (tempo);          \
            }                                                                     \
            if ((state)->tremolo_length > 0 && (state)->tremolo_depth > 0)        \
            {                                                                     \
                (state)->tremolo_phase *= (double)(freq) / (state)->freq;         \
                (state)->tremolo_phase *= (state)->tempo / (tempo);               \
            }                                                                     \
            (state)->tremolo_length *= (double)(freq) / (state)->freq;            \
            (state)->tremolo_length *= (state)->tempo / (tempo);                  \
            (state)->tremolo_update *= (double)(state)->freq / (freq);            \
            (state)->tremolo_update *= (tempo) / (state)->tempo;                  \
            (state)->tremolo_delay_update *= (double)(state)->freq / (freq);      \
            (state)->tremolo_delay_update *= (tempo) / (state)->tempo;            \
            if ((state)->panning_slide != 0)                                      \
            {                                                                     \
                (state)->panning_slide_update *= (double)(state)->freq / (freq);  \
                (state)->panning_slide_update *= (tempo) / (state)->tempo;        \
                (state)->panning_slide_frames *= (double)(freq) / (state)->freq;  \
                (state)->panning_slide_frames *= (state)->tempo / (tempo);        \
            }                                                                     \
            if ((state)->filter_slide != 0)                                       \
            {                                                                     \
                double slide_step = log2((state)->filter_slide_update);           \
                slide_step *= (double)(state)->freq / (freq);                     \
                slide_step *= (tempo) / (state)->tempo;                           \
                (state)->filter_slide_update = exp2(slide_step);                  \
                (state)->filter_slide_frames *= (double)(freq) / (state)->freq;   \
                (state)->filter_slide_frames *= (state)->tempo / (tempo);         \
            }                                                                     \
            if ((state)->autowah_length > 0 && (state)->autowah_depth > 0)        \
            {                                                                     \
                (state)->autowah_phase *= (double)(freq) / (state)->freq;         \
                (state)->autowah_phase *= (state)->tempo / (tempo);               \
            }                                                                     \
            (state)->autowah_length *= (double)(freq) / (state)->freq;            \
            (state)->autowah_length *= (state)->tempo / (tempo);                  \
            (state)->autowah_update *= (double)(state)->freq / (freq);            \
            (state)->autowah_update *= (tempo) / (state)->tempo;                  \
            (state)->autowah_delay_update *= (double)(state)->freq / (freq);      \
            (state)->autowah_delay_update *= (tempo) / (state)->tempo;            \
                                                                                  \
            (state)->freq = (freq);                                               \
            (state)->tempo = (tempo);                                             \
        }                                                                         \
    } else (void)0


/**
 * Filter handling.
 * This code doesn't actually modify the sound but merely controls
 * higher-level parameters and breaks the mixing cycle in case filter
 * coefficients need to be updated. It can therefore be called before any
 * inital amplitude values are calculated.
 */
#define Generator_common_handle_filter(gen, state)                                     \
    if (true)                                                                          \
    {                                                                                  \
        if (!(state)->filter_update &&                                                 \
                ((state)->actual_filter < (state)->effective_filter * 0.98566319864018759   \
                 || (state)->actual_filter > (state)->effective_filter * 1.0145453349375237 \
                 || (state)->filter_resonance != (state)->effective_resonance))        \
        {                                                                              \
            (state)->filter_update = true;                                             \
            break;                                                                     \
        }                                                                              \
        if ((state)->filter_slide != 0)                                                \
        {                                                                              \
            (state)->filter *= (state)->filter_slide_update;                           \
            (state)->filter_slide_frames -= 1;                                         \
            if ((state)->filter_slide_frames <= 0)                                     \
            {                                                                          \
                (state)->filter = (state)->filter_slide_target;                        \
                (state)->filter_slide = 0;                                             \
            }                                                                          \
            else if ((state)->filter_slide == 1)                                       \
            {                                                                          \
                if ((state)->filter > (state)->filter_slide_target)                    \
                {                                                                      \
                    (state)->filter = (state)->filter_slide_target;                    \
                    (state)->filter_slide = 0;                                         \
                }                                                                      \
            }                                                                          \
            else                                                                       \
            {                                                                          \
                assert((state)->filter_slide == -1);                                   \
                if ((state)->filter < (state)->filter_slide_target)                    \
                {                                                                      \
                    (state)->filter = (state)->filter_slide_target;                    \
                    (state)->filter_slide = 0;                                         \
                }                                                                      \
            }                                                                          \
        }                                                                              \
        (state)->actual_filter = (state)->filter;                                      \
        if ((state)->autowah)                                                          \
        {                                                                              \
            double fac_log = sin((state)->autowah_phase);                              \
            if ((state)->autowah_delay_pos < 1)                                        \
            {                                                                          \
                double actual_depth = (1 - (state)->autowah_delay_pos) *               \
                        (state)->autowah_depth +                                       \
                        (state)->autowah_delay_pos *                                   \
                        (state)->autowah_depth_target;                                 \
                fac_log *= actual_depth;                                               \
                (state)->autowah_delay_pos += (state)->autowah_delay_update;           \
            }                                                                          \
            else                                                                       \
            {                                                                          \
                (state)->autowah_depth = (state)->autowah_depth_target;                \
                fac_log *= (state)->autowah_depth;                                     \
                if ((state)->autowah_depth == 0)                                       \
                {                                                                      \
                    (state)->autowah = false;                                          \
                }                                                                      \
            }                                                                          \
            if (fac_log > 85)                                                          \
            {                                                                          \
                fac_log = 85;                                                          \
            }                                                                          \
            (state)->actual_filter *= exp2(fac_log);                                   \
            if (!(state)->autowah &&                                                   \
                    (state)->autowah_length > (state)->freq)                           \
            {                                                                          \
                (state)->autowah_length = (state)->freq;                               \
                (state)->autowah_update = (2 * PI) / (state)->autowah_length;          \
            }                                                                          \
            double new_phase = (state)->autowah_phase +                                \
                    (state)->autowah_update;                                           \
            if (new_phase >= (2 * PI))                                                 \
            {                                                                          \
                new_phase = fmod(new_phase, (2 * PI));                                 \
            }                                                                          \
            if (!(state)->autowah && (new_phase < (state)->autowah_phase               \
                        || (new_phase >= PI && (state)->autowah_phase < PI)))          \
            {                                                                          \
                (state)->autowah_phase = 0;                                            \
                (state)->autowah_update = 0;                                           \
            }                                                                          \
            else                                                                       \
            {                                                                          \
                (state)->autowah_phase = new_phase;                                    \
            }                                                                          \
        }                                                                              \
    } else (void)0


/**
 * Pitch handling.
 */
#define Generator_common_handle_pitch(gen, state)                             \
    if (true)                                                                 \
    {                                                                         \
        if ((state)->pitch_slide != 0)                                        \
        {                                                                     \
            (state)->pitch *= (state)->pitch_slide_update;                    \
            (state)->pitch_slide_frames -= 1;                                 \
            if ((state)->pitch_slide_frames <= 0)                             \
            {                                                                 \
                (state)->pitch = (state)->pitch_slide_target;                 \
                (state)->pitch_slide = 0;                                     \
            }                                                                 \
            else if ((state)->pitch_slide == 1)                               \
            {                                                                 \
                if ((state)->pitch > (state)->pitch_slide_target)             \
                {                                                             \
                    (state)->pitch = (state)->pitch_slide_target;             \
                    (state)->pitch_slide = 0;                                 \
                }                                                             \
            }                                                                 \
            else                                                              \
            {                                                                 \
                assert((state)->pitch_slide == -1);                           \
                if ((state)->pitch < (state)->pitch_slide_target)             \
                {                                                             \
                    (state)->pitch = (state)->pitch_slide_target;             \
                    (state)->pitch_slide = 0;                                 \
                }                                                             \
            }                                                                 \
        }                                                                     \
        (state)->actual_pitch = (state)->pitch;                               \
        if ((state)->vibrato)                                                 \
        {                                                                     \
            double fac_log = sin((state)->vibrato_phase);                     \
            if ((state)->vibrato_delay_pos < 1)                               \
            {                                                                 \
                double actual_depth = (1 - (state)->vibrato_delay_pos) *      \
                        (state)->vibrato_depth +                              \
                        (state)->vibrato_delay_pos *                          \
                        (state)->vibrato_depth_target;                        \
                fac_log *= actual_depth;                                      \
                (state)->vibrato_delay_pos += (state)->vibrato_delay_update;  \
            }                                                                 \
            else                                                              \
            {                                                                 \
                (state)->vibrato_depth = (state)->vibrato_depth_target;       \
                fac_log *= (state)->vibrato_depth;                            \
                if ((state)->vibrato_depth == 0)                              \
                {                                                             \
                    (state)->vibrato = false;                                 \
                }                                                             \
            }                                                                 \
            (state)->actual_pitch *= exp2(fac_log);                           \
            if (!(state)->vibrato &&                                          \
                    (state)->vibrato_length > (state)->freq)                  \
            {                                                                 \
                (state)->vibrato_length = (state)->freq;                      \
                (state)->vibrato_update = (2 * PI) / (state)->vibrato_length; \
            }                                                                 \
            double new_phase = (state)->vibrato_phase +                       \
                    (state)->vibrato_update;                                  \
            if (new_phase >= (2 * PI))                                        \
            {                                                                 \
                new_phase = fmod(new_phase, (2 * PI));                        \
            }                                                                 \
            if (!(state)->vibrato && (new_phase < (state)->vibrato_phase      \
                    || (new_phase >= PI && (state)->vibrato_phase < PI)))     \
            {                                                                 \
                (state)->vibrato_phase = 0;                                   \
                (state)->vibrato_update = 0;                                  \
            }                                                                 \
            else                                                              \
            {                                                                 \
                (state)->vibrato_phase = new_phase;                           \
            }                                                                 \
        }                                                                     \
        if ((state)->arpeggio)                                                \
        {                                                                     \
            if ((state)->arpeggio_note > 0)                                   \
            {                                                                 \
                (state)->actual_pitch *= (state)->arpeggio_factors[           \
                        (state)->arpeggio_note - 1];                          \
            }                                                                 \
            (state)->arpeggio_frames += 1;                                    \
            if ((state)->arpeggio_frames >= (state)->arpeggio_length)         \
            {                                                                 \
                (state)->arpeggio_frames -= (state)->arpeggio_length;         \
                ++(state)->arpeggio_note;                                     \
                if ((state)->arpeggio_note > KQT_ARPEGGIO_NOTES_MAX           \
                        || (state)->arpeggio_factors[                         \
                                (state)->arpeggio_note - 1] <= 0)             \
                {                                                             \
                    (state)->arpeggio_note = 0;                               \
                }                                                             \
            }                                                                 \
        }                                                                     \
    } else (void)0


/**
 * Force handling.
 * This may affect the actual pitch and filter cutoff settings and therefore
 * should be called after Generator_common_handle_pitch and
 * Generator_common_handle_filter.
 */
#define Generator_common_handle_force(gen, state, frames, frame_count)              \
    if (true)                                                                       \
    {                                                                               \
        if ((state)->force_slide != 0)                                              \
        {                                                                           \
            (state)->force *= (state)->force_slide_update;                          \
            (state)->force_slide_frames -= 1;                                       \
            if ((state)->force_slide_frames <= 0)                                   \
            {                                                                       \
                (state)->force = (state)->force_slide_target;                       \
                (state)->force_slide = 0;                                           \
            }                                                                       \
            else if ((state)->force_slide == 1)                                     \
            {                                                                       \
                if ((state)->force > (state)->force_slide_target)                   \
                {                                                                   \
                    (state)->force = (state)->force_slide_target;                   \
                    (state)->force_slide = 0;                                       \
                }                                                                   \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                assert((state)->force_slide == -1);                                 \
                if ((state)->force < (state)->force_slide_target)                   \
                {                                                                   \
                    (state)->force = (state)->force_slide_target;                   \
                    (state)->force_slide = 0;                                       \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        (state)->actual_force = (state)->force;                                     \
        if ((state)->tremolo)                                                       \
        {                                                                           \
            double fac_dB = sin((state)->tremolo_phase);                            \
            if ((state)->tremolo_delay_pos < 1)                                     \
            {                                                                       \
                double actual_depth = (1 - (state)->tremolo_delay_pos) *            \
                        (state)->tremolo_depth +                                    \
                        (state)->tremolo_delay_pos *                                \
                        (state)->tremolo_depth_target;                              \
                fac_dB *= actual_depth;                                             \
                (state)->tremolo_delay_pos += (state)->tremolo_delay_update;        \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                (state)->tremolo_depth = (state)->tremolo_depth_target;             \
                fac_dB *= (state)->tremolo_depth;                                   \
                if ((state)->tremolo_depth == 0)                                    \
                {                                                                   \
                    (state)->tremolo = false;                                       \
                }                                                                   \
            }                                                                       \
            (state)->actual_force *= exp2(fac_dB / 6);                              \
            if (!(state)->tremolo &&                                                \
                    (state)->tremolo_length > (state)->freq)                        \
            {                                                                       \
                (state)->tremolo_length = (state)->freq;                            \
                (state)->tremolo_update = (2 * PI) / (state)->tremolo_length;       \
            }                                                                       \
            double new_phase = (state)->tremolo_phase +                             \
                    (state)->tremolo_update;                                        \
            if (new_phase >= (2 * PI))                                              \
            {                                                                       \
                new_phase = fmod(new_phase, (2 * PI));                              \
            }                                                                       \
            if (!(state)->tremolo && (new_phase < (state)->tremolo_phase            \
                        || (new_phase >= PI && (state)->tremolo_phase < PI)))       \
            {                                                                       \
                (state)->tremolo_phase = 0;                                         \
                (state)->tremolo_update = 0;                                        \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                (state)->tremolo_phase = new_phase;                                 \
            }                                                                       \
        }                                                                           \
        if ((gen)->ins_params->env_force_enabled)                                   \
        {                                                                           \
            Envelope* env = (gen)->ins_params->env_force;                           \
            double scale = Envelope_get_value(env, (state)->fe_pos);                \
            assert(isfinite(scale));                                                \
            (state)->actual_force *= scale;                                         \
            int loop_start_index = Envelope_get_mark(env, 0);                       \
            int loop_end_index = Envelope_get_mark(env, 1);                         \
            double* loop_start = loop_start_index == -1 ? NULL :                    \
                                 Envelope_get_node(env, loop_start_index);          \
            double* loop_end = loop_end_index == -1 ? NULL :                        \
                               Envelope_get_node(env, loop_end_index);              \
            double stretch = 1;                                                     \
            if ((gen)->ins_params->env_force_scale_amount != 0)                     \
            {                                                                       \
                stretch = pow((state)->actual_pitch /                               \
                                  (gen)->ins_params->env_force_center,              \
                              (gen)->ins_params->env_force_scale_amount);           \
            }                                                                       \
            double new_pos = (state)->fe_pos + (stretch) / (freq);                  \
            if (loop_start != NULL && loop_end != NULL)                             \
            {                                                                       \
                if (new_pos > loop_end[0])                                          \
                {                                                                   \
                    double loop_len = loop_end[0] - loop_start[0];                  \
                    assert(loop_len >= 0);                                          \
                    if (loop_len == 0)                                              \
                    {                                                               \
                        new_pos = loop_end[0];                                      \
                    }                                                               \
                    else                                                            \
                    {                                                               \
                        double exceed = new_pos - loop_end[0];                      \
                        double offset = fmod(exceed, loop_len);                     \
                        new_pos = loop_start[0] + offset;                           \
                        assert(new_pos >= loop_start[0]);                           \
                        assert(new_pos <= loop_end[0]);                             \
                    }                                                               \
                }                                                                   \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                double* last = Envelope_get_node(env,                               \
                                                 Envelope_node_count(env) - 1);     \
                if (new_pos > last[0])                                              \
                {                                                                   \
                    new_pos = last[0];                                              \
                    if ((state)->fe_pos > last[0] && last[1] == 0)                  \
                    {                                                               \
                        (state)->active = false;                                    \
                        break;                                                      \
                    }                                                               \
                }                                                                   \
            }                                                                       \
            (state)->fe_pos = new_pos;                                              \
        }                                                                           \
        if (!(state)->note_on)                                                      \
        {                                                                           \
            if ((gen)->ins_params->env_force_rel_enabled)                           \
            {                                                                       \
                double scale = Envelope_get_value((gen)->ins_params->env_force_rel, \
                                                  (state)->rel_fe_pos);             \
                if (!isfinite(scale))                                               \
                {                                                                   \
                    (state)->active = false;                                        \
                    break;                                                          \
                }                                                                   \
                double stretch = 1;                                                 \
                if ((gen)->ins_params->env_force_rel_scale_amount != 0)             \
                {                                                                   \
                    stretch = pow((state)->actual_pitch /                           \
                                      (gen)->ins_params->env_force_rel_center,      \
                                  (gen)->ins_params->env_force_rel_scale_amount);   \
                }                                                                   \
                (state)->rel_fe_pos += stretch * (1.0 - *(state)->pedal) / (freq);  \
                (state)->actual_force *= scale;                                     \
            }                                                                       \
            else if (*(state)->pedal < 0.5)                                         \
            {                                                                       \
                if ((state)->ramp_release < 1)                                      \
                {                                                                   \
                    for (int i = 0; i < (frame_count); ++i)                         \
                    {                                                               \
                        (frames)[i] *= 1 - (state)->ramp_release;                   \
                    }                                                               \
                }                                                                   \
                else                                                                \
                {                                                                   \
                    (state)->active = false;                                        \
                    break;                                                          \
                }                                                                   \
                (state)->ramp_release += RAMP_RELEASE_TIME / (freq);                \
            }                                                                       \
        }                                                                           \
        if ((gen)->ins_params->env_force_filter_enabled)                            \
        {                                                                           \
            double force = (state)->actual_force;                                   \
            if (force > 1)                                                          \
            {                                                                       \
                force = 1;                                                          \
            }                                                                       \
            double factor = Envelope_get_value((gen)->ins_params->env_force_filter, \
                                               force);                              \
            assert(isfinite(factor));                                               \
            (state)->actual_filter = MIN((state)->actual_filter, 16384) * factor;   \
        }                                                                           \
        for (int i = 0; i < (frame_count); ++i)                                     \
        {                                                                           \
            (frames)[i] *= (state)->actual_force;                                   \
        }                                                                           \
    } else (void)0


/**
 * Automatic volume ramping for note start and end.
 * This should be called after force handling if needed (not all Generators
 * need this).
 */
#define Generator_common_ramp_attack(gen, state, frames, frame_count, freq) \
    if (true)                                                               \
    {                                                                       \
        if ((state)->ramp_attack < 1)                                       \
        {                                                                   \
            for (int i = 0; i < (frame_count); ++i)                         \
            {                                                               \
                (frames)[i] *= (state)->ramp_attack;                        \
            }                                                               \
            (state)->ramp_attack += RAMP_ATTACK_TIME / (freq);              \
        }                                                                   \
    } else (void)0


/**
 * Note Off handling (includes handling Note Off envelopes and breaking if
 * needed).
 * This should be called after updating the Voice state position indicator.
 */
#if 0
#define Generator_common_handle_note_off(gen, state, frames, frame_count, freq)     \
    if (true)                                                                       \
    {                                                                               \
        if (!(state)->note_on)                                                      \
        {                                                                           \
            if ((gen)->ins_params->env_force_rel_enabled)                           \
            {                                                                       \
                double scale = Envelope_get_value((gen)->ins_params->env_force_rel, \
                                                  (state)->rel_fe_pos);             \
                if (!isfinite(scale))                                               \
                {                                                                   \
                    (state)->active = false;                                        \
                    break;                                                          \
                }                                                                   \
                (state)->rel_fe_pos += (1.0 - *(state)->pedal) / (freq);            \
                for (int i = 0; i < (frame_count); ++i)                             \
                {                                                                   \
                    (frames)[i] *= scale;                                           \
                }                                                                   \
            }                                                                       \
            else if (*(state)->pedal < 0.5)                                         \
            {                                                                       \
                if ((state)->ramp_release < 1)                                      \
                {                                                                   \
                    for (int i = 0; i < (frame_count); ++i)                         \
                    {                                                               \
                        (frames)[i] *= 1 - (state)->ramp_release;                   \
                    }                                                               \
                }                                                                   \
                else                                                                \
                {                                                                   \
                    (state)->active = false;                                        \
                    break;                                                          \
                }                                                                   \
                (state)->ramp_release += RAMP_RELEASE_TIME / (freq);                \
            }                                                                       \
        }                                                                           \
    } else (void)0
#endif


/**
 * Panning handling.
 */
#define Generator_common_handle_panning(gen, state, frames, frame_count)  \
    if (true)                                                             \
    {                                                                     \
        if ((frame_count) >= 2)                                           \
        {                                                                 \
            if ((state)->panning_slide != 0)                              \
            {                                                             \
                (state)->panning += (state)->panning_slide_update;        \
                (state)->panning_slide_frames -= 1;                       \
                if ((state)->panning_slide_frames <= 0)                   \
                {                                                         \
                    (state)->panning = (state)->panning_slide_target;     \
                    (state)->panning_slide = 0;                           \
                }                                                         \
                else if ((state)->panning_slide == 1)                     \
                {                                                         \
                    if ((state)->panning > (state)->panning_slide_target) \
                    {                                                     \
                        (state)->panning = (state)->panning_slide_target; \
                        (state)->panning_slide = 0;                       \
                    }                                                     \
                }                                                         \
                else                                                      \
                {                                                         \
                    assert((state)->panning_slide == -1);                 \
                    if ((state)->panning < (state)->panning_slide_target) \
                    {                                                     \
                        (state)->panning = (state)->panning_slide_target; \
                        (state)->panning_slide = 0;                       \
                    }                                                     \
                }                                                         \
            }                                                             \
            (state)->actual_panning = (state)->panning;                   \
            (frames)[0] *= 1 - (state)->actual_panning;                   \
            (frames)[1] *= 1 + (state)->actual_panning;                   \
        }                                                                 \
    } else (void)0


