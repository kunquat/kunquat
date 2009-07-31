

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


#ifndef K_VOICE_STATE_H
#define K_VOICE_STATE_H


#include <stdbool.h>
#include <stdint.h>

#include <pitch_t.h>


typedef struct Voice_state
{
    bool active;                 ///< Whether there is anything left to process.
    uint32_t mix_freq;           ///< The last mixing frequency used.
    double tempo;                ///< The last tempo setting used.
    double ramp_attack;          ///< The current state of volume ramp during attack.
    double ramp_release;         ///< The current state of volume ramp during release.
                                
    pitch_t freq;                ///< The frequency at which the note is played.
                                
    uint64_t pos;                ///< The current playback position.
    double pos_rem;              ///< The current playback position remainder.
    uint64_t rel_pos;            ///< The current relative playback position.
    double rel_pos_rem;          ///< The current relative playback position remainder.
    double dir;                  ///< The current playback direction.
    bool note_on;                ///< Whether the note is still on.
    uint64_t noff_pos;           ///< Note Off position.
    double noff_pos_rem;         ///< Note Off position remainder.

    bool pedal;                  ///< Whether the pedal is active.
    double on_ve_pos;            ///< Note On volume envelope position.
    double off_ve_pos;           ///< Note Off volume envelope position.
                                
    double force;                ///< The current force (linear factor).
    int force_slide;             ///< Force slide state (0 = no slide, -1 = down, 1 = up).
    double force_slide_target;   ///< Target force of the slide.
    double force_slide_frames;   ///< Number of frames left to complete the slide.
    double force_slide_update;   ///< The update factor of the slide.
} Voice_state;


/**
 * Initialises a Voice state.
 *
 * \param state   The Voice state -- must not be \c NULL.
 * \param freq    The mixing frequency -- must be > \c 0.
 * \param tempo   The current tempo -- must be > \c 0.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_init(Voice_state* state, uint32_t freq, double tempo);


/**
 * Clears a Voice state.
 * \param state   The Voice state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_clear(Voice_state* state);


#endif // K_VOICE_STATE_H


