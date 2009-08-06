

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <Voice_state.h>
#include <Generator.h>
#include <kunquat/frame.h>
#include <kunquat/limits.h>
#include <math_common.h>


#define RAMP_ATTACK_TIME (500.0)
#define RAMP_RELEASE_TIME (200.0)


#define Generator_common_check_active(gen, state, mixed)                      \
    if (true)                                                                 \
    {                                                                         \
        if (!(state)->active || (!(state)->note_on &&                         \
                                 ((state)->pos == 0) &&                       \
                                 ((state)->pos_rem == 0) &&                   \
                                 !(gen)->ins_params->volume_off_env_enabled)) \
        {                                                                     \
            (state)->active = false;                                          \
            return (mixed);                                                   \
        }                                                                     \
    } else (void)0


#define Generator_common_check_relative_lengths(gen, state, freq, tempo)          \
    if (true)                                                                     \
    {                                                                             \
        if ((state)->freq != (freq) || (state)->tempo != (tempo))                 \
        {                                                                         \
            if ((state)->pitch_slide != 0)                                        \
            {                                                                     \
                double slide_step = log2((state)->pitch_slide_update);            \
                slide_step = slide_step * (state)->freq / (freq);                 \
                slide_step = slide_step * (tempo) / (state)->tempo;               \
                (state)->pitch_slide_update = exp2(slide_step);                   \
                (state)->pitch_slide_frames *= (freq) / (state)->freq;            \
                (state)->pitch_slide_frames *= (state)->tempo / (tempo);          \
            }                                                                     \
            if ((state)->vibrato_length > 0 && (state)->vibrato_depth > 0)        \
            {                                                                     \
                (state)->vibrato_length *= (freq) / (state)->freq;                \
                (state)->vibrato_length *= (state)->tempo / (tempo);              \
                (state)->vibrato_phase *= (freq) / (state)->freq;                 \
                (state)->vibrato_phase *= (state)->tempo / (tempo);               \
                (state)->vibrato_update *= (state)->freq / (freq);                \
                (state)->vibrato_update *= (tempo) / (state)->tempo;              \
            }                                                                     \
            if ((state)->arpeggio)                                                \
            {                                                                     \
                (state)->arpeggio_length *= (freq) / (state)->freq;               \
                (state)->arpeggio_length *= (state)->tempo / (tempo);             \
                (state)->arpeggio_frames *= (freq) / (state)->freq;               \
                (state)->arpeggio_frames *= (state)->tempo / (tempo);             \
            }                                                                     \
            if ((state)->force_slide != 0)                                        \
            {                                                                     \
                double update_dB = log2((state)->force_slide_update) * 6;         \
                update_dB *= (state)->freq / (freq);                              \
                update_dB *= (tempo) / (state)->tempo;                            \
                (state)->force_slide_update = exp2(update_dB / 6);                \
                (state)->force_slide_frames *= (freq) / (state)->freq;            \
                (state)->force_slide_frames *= (state)->tempo / (tempo);          \
            }                                                                     \
            if ((state)->tremolo_length > 0 && (state)->tremolo_depth > 0)        \
            {                                                                     \
                (state)->tremolo_length *= (freq) / (state)->freq;                \
                (state)->tremolo_length *= (state)->tempo / (tempo);              \
                (state)->tremolo_phase *= (freq) / (state)->freq;                 \
                (state)->tremolo_phase *= (state)->tempo / (tempo);               \
                (state)->tremolo_update *= (state)->freq / (freq);                \
                (state)->tremolo_update *= (tempo) / (state)->tempo;              \
            }                                                                     \
            if ((state)->panning_slide != 0)                                      \
            {                                                                     \
                (state)->panning_slide_update *= (state)->freq / (freq);          \
                (state)->panning_slide_update *= (tempo) / (state)->tempo;        \
                (state)->panning_slide_frames *= (freq) / (state)->freq;          \
                (state)->panning_slide_frames *= (state)->tempo / (tempo);        \
            }                                                                     \
            (state)->freq = (freq);                                               \
            (state)->tempo = (tempo);                                             \
        }                                                                         \
    } else (void)0


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


#define Generator_common_handle_note_off(gen, state, frames, frame_count, freq)      \
    if (true)                                                                        \
    {                                                                                \
        if (!(state)->note_on)                                                       \
        {                                                                            \
            if ((gen)->ins_params->volume_off_env_enabled)                           \
            {                                                                        \
                double scale = Envelope_get_value((gen)->ins_params->volume_off_env, \
                                                  (state)->off_ve_pos);              \
                if (!isfinite(scale))                                                \
                {                                                                    \
                    (state)->active = false;                                         \
                    break;                                                           \
                }                                                                    \
                (state)->off_ve_pos += (1.0 - (state)->pedal) / (freq);              \
                for (int i = 0; i < (frame_count); ++i)                              \
                {                                                                    \
                    (frames)[i] *= scale;                                            \
                }                                                                    \
            }                                                                        \
            else                                                                     \
            {                                                                        \
                if ((state)->ramp_release < 1)                                       \
                {                                                                    \
                    for (int i = 0; i < (frame_count); ++i)                          \
                    {                                                                \
                        (frames)[i] *= 1 - (state)->ramp_release;                    \
                    }                                                                \
                }                                                                    \
                else                                                                 \
                {                                                                    \
                    (state)->active = false;                                         \
                    break;                                                           \
                }                                                                    \
                (state)->ramp_release += RAMP_RELEASE_TIME / (freq);                 \
            }                                                                        \
        }                                                                            \
    } else (void)0


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
            double fac_log = sin((state)->vibrato_phase) *                    \
                    (state)->vibrato_depth;                                   \
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
                (state)->vibrato_length = 0;                                  \
                (state)->vibrato_depth = 0;                                   \
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


#define Generator_common_handle_force(gen, state, frames, frame_count)        \
    if (true)                                                                 \
    {                                                                         \
        if ((state)->force_slide != 0)                                        \
        {                                                                     \
            (state)->force *= (state)->force_slide_update;                    \
            (state)->force_slide_frames -= 1;                                 \
            if ((state)->force_slide_frames <= 0)                             \
            {                                                                 \
                (state)->force = (state)->force_slide_target;                 \
                (state)->force_slide = 0;                                     \
            }                                                                 \
            else if ((state)->force_slide == 1)                               \
            {                                                                 \
                if ((state)->force > (state)->force_slide_target)             \
                {                                                             \
                    (state)->force = (state)->force_slide_target;             \
                    (state)->force_slide = 0;                                 \
                }                                                             \
            }                                                                 \
            else                                                              \
            {                                                                 \
                assert((state)->force_slide == -1);                           \
                if ((state)->force < (state)->force_slide_target)             \
                {                                                             \
                    (state)->force = (state)->force_slide_target;             \
                    (state)->force_slide = 0;                                 \
                }                                                             \
            }                                                                 \
        }                                                                     \
        (state)->actual_force = (state)->force;                               \
        if ((state)->tremolo_length > 0 && (state)->tremolo_depth > 0)        \
        {                                                                     \
            double fac_dB = sin((state)->tremolo_phase) *                     \
                    (state)->tremolo_depth;                                   \
            (state)->actual_force *= exp2(fac_dB / 6);                        \
            if (!(state)->tremolo &&                                          \
                    (state)->tremolo_length > (state)->freq)                  \
            {                                                                 \
                (state)->tremolo_length = (state)->freq;                      \
                (state)->tremolo_update = (2 * PI) / (state)->tremolo_length; \
            }                                                                 \
            double new_phase = (state)->tremolo_phase +                       \
                    (state)->tremolo_update;                                  \
            if (new_phase >= (2 * PI))                                        \
            {                                                                 \
                new_phase = fmod(new_phase, (2 * PI));                        \
            }                                                                 \
            if (!(state)->tremolo && (new_phase < (state)->tremolo_phase      \
                        || (new_phase >= PI && (state)->tremolo_phase < PI))) \
            {                                                                 \
                (state)->tremolo_length = 0;                                  \
                (state)->tremolo_depth = 0;                                   \
                (state)->tremolo_phase = 0;                                   \
                (state)->tremolo_update = 0;                                  \
            }                                                                 \
            else                                                              \
            {                                                                 \
                (state)->tremolo_phase = new_phase;                           \
            }                                                                 \
        }                                                                     \
        for (int i = 0; i < (frame_count); ++i)                               \
        {                                                                     \
            (frames)[i] *= (state)->actual_force;                             \
        }                                                                     \
    } else (void)0


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


#define Generator_common_persist(gen, state, mixed)                         \
    if (true)                                                               \
    {                                                                       \
        Channel_state* ch_state = (state)->new_ch_state;                    \
        if ((state)->note_on && (mixed) > ch_state->panning_slide_prog)     \
        {                                                                   \
            ch_state->panning_slide_prog = (mixed);                         \
            ch_state->panning = (state)->panning;                           \
            ch_state->panning_slide = (state)->panning_slide;               \
            ch_state->panning_slide_target = (state)->panning_slide_target; \
            ch_state->panning_slide_frames = (state)->panning_slide_frames; \
            ch_state->panning_slide_update = (state)->panning_slide_update; \
        }                                                                   \
    } else (void)0


