

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
 *
 * This should be called before the mixing loop of the Generator.
 *
 * \param gen     The Generator -- must not be \c NULL.
 * \param state   The Voice state -- must not be \c NULL.
 * \param freq    The mixing frequency -- must be > \c 0.
 * \param tempo   The tempo -- must be > \c 0 and finite.
 */
void Generator_common_check_relative_lengths(Generator* gen,
                                             Voice_state* state,
                                             uint32_t freq,
                                             double tempo);


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
 *
 * \param gen     The Generator -- must not be \c NULL.
 * \param state   The Voice state -- must not be \c NULL.
 */
void Generator_common_handle_pitch(Generator* gen,
                                   Voice_state* state);


/**
 * Force handling.
 * 
 * This may affect the actual pitch and filter cutoff settings and therefore
 * should be called after Generator_common_handle_pitch and
 * Generator_common_handle_filter.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_handle_force(Generator* gen,
                                   Voice_state* state,
                                   double frames[],
                                   int frame_count,
                                   uint32_t freq);


/**
 * Automatic volume ramping for note start and end.
 *
 * This should be called after force handling if needed (not all Generators
 * need this).
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_ramp_attack(Generator* gen,
                                  Voice_state* state,
                                  double frames[],
                                  int frame_count,
                                  uint32_t freq);


/**
 * Panning handling.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 */
void Generator_common_handle_panning(Generator* gen,
                                     Voice_state* state,
                                     double frames[],
                                     int frame_count);


