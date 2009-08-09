

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

#include <Channel_state.h>
#include <kunquat/frame.h>
#include <kunquat/limits.h>
#include <pitch_t.h>


#define FILTER_ORDER (4)


typedef struct Filter_state
{
    double coeffs1[FILTER_ORDER]; ///< First coefficient table.
    double coeffs2[FILTER_ORDER + 1]; ///< Second coefficient table.
    kqt_frame history1[KQT_BUFFERS_MAX][FILTER_ORDER]; ///< History buffer.
    kqt_frame history2[KQT_BUFFERS_MAX][FILTER_ORDER]; ///< History buffer.
} Filter_state;


typedef struct Voice_state
{
    bool active;                   ///< Whether there is anything left to process.
    uint32_t freq;                 ///< The last mixing frequency used.
    double tempo;                  ///< The last tempo setting used.
    const Channel_state* cur_ch_state;   ///< Current Channel state.
    Channel_state* new_ch_state;   ///< New (upcoming) Channel state.

    double ramp_attack;            ///< The current state of volume ramp during attack.
    double ramp_release;           ///< The current state of volume ramp during release.
    int orig_note;                 ///< The note used at the beginning.
    int orig_note_mod;             ///< The note modifier used at the beginning.
    int orig_octave;               ///< The octave used at the beginning.
                                  
    pitch_t pitch;                 ///< The frequency at which the note is played.
    pitch_t actual_pitch;          ///< The actual frequency (includes vibrato).
    int pitch_slide;               ///< Pitch slide state (0 = no slide, -1 = down, 1 = up).
    pitch_t pitch_slide_target;    ///< Target pitch of the slide.
    double pitch_slide_frames;     ///< Number of frames left to complete the slide.
    double pitch_slide_update;     ///< The update factor of the slide.
    bool vibrato;                  ///< Vibrato enabled.
    double vibrato_length;         ///< Length of the vibrato phase.
    double vibrato_depth;          ///< Depth of the vibrato.
    double vibrato_depth_target;   ///< Target vibrato depth.
    double vibrato_delay_pos;      ///< Position of the vibrato delay.
    double vibrato_delay_update;   ///< The update amount of the vibrato delay.
    double vibrato_phase;          ///< Phase of the vibrato.
    double vibrato_update;         ///< The update amount of the vibrato phase.
    bool arpeggio;                 ///< Arpeggio enabled.
    double arpeggio_length;        ///< Length of one note in the arpeggio.
    double arpeggio_frames;        ///< Frames left of the current note in the arpeggio.
    int arpeggio_note;             ///< Current note in the arpeggio.
    double arpeggio_factors[KQT_ARPEGGIO_NOTES_MAX]; ///< Pitch factors in the arpeggio.
                                 
    uint64_t pos;                  ///< The current playback position.
    double pos_rem;                ///< The current playback position remainder.
    uint64_t rel_pos;              ///< The current relative playback position.
    double rel_pos_rem;            ///< The current relative playback position remainder.
    double dir;                    ///< The current playback direction.
    bool note_on;                  ///< Whether the note is still on.
    uint64_t noff_pos;             ///< Note Off position.
    double noff_pos_rem;           ///< Note Off position remainder.
                                  
    bool pedal;                    ///< Whether the pedal is active.
    double on_ve_pos;              ///< Note On volume envelope position.
    double off_ve_pos;             ///< Note Off volume envelope position.
                                  
    double force;                  ///< The current force (linear factor).
    double actual_force;           ///< The current actual force (includes tremolo).
    int force_slide;               ///< Force slide state (0 = no slide, -1 = down, 1 = up).
    double force_slide_target;     ///< Target force of the slide.
    double force_slide_frames;     ///< Number of frames left to complete the slide.
    double force_slide_update;     ///< The update factor of the slide.
    bool tremolo;                  ///< Tremolo enabled.
    double tremolo_length;         ///< Length of the tremolo phase.
    double tremolo_depth;          ///< Depth of the tremolo.
    double tremolo_depth_target;   ///< Target tremolo depth.
    double tremolo_delay_pos;      ///< Position of the tremolo delay.
    double tremolo_delay_update;   ///< The update amount of the tremolo delay.
    double tremolo_phase;          ///< Phase of the tremolo.
    double tremolo_update;         ///< The update amount of the tremolo phase.
                                   
    double panning;                ///< The current panning.
    double actual_panning;         ///< The current actual panning.
    int panning_slide;             ///< Panning slide state (0 = no slide, -1 = left, 1 = right).
    double panning_slide_target;   ///< Target panning position of the slide.
    double panning_slide_frames;   ///< Number of frames left to complete the slide.
    double panning_slide_update;   ///< The update amount of the slide.

    double filter;                 ///< The current filter cut-off frequency.
    double actual_filter;          ///< The current actual filter cut-off frequency.
    bool filter_update;            ///< Whether filter needs to be updated.
    double filter_xfade_pos;       ///< Filter crossfade position.
    double filter_xfade_update;    ///< The update amount of the filter crossfade.
    int filter_xfade_state_used;   ///< State fading out during the filter crossfade.
    int filter_state_used;         ///< Primary filter state used.
    Filter_state filter_state[2];  ///< States of the filters.
} Voice_state;


/**
 * Initialises a Voice state.
 *
 * \param state          The Voice state -- must not be \c NULL.
 * \param cur_ch_state   The current Channel state -- must not be \c NULL.
 * \param new_ch_state   The new (upcoming) Channel state -- must not be \c NULL.
 * \param freq           The mixing frequency -- must be > \c 0.
 * \param tempo          The current tempo -- must be > \c 0.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_init(Voice_state* state,
                              const Channel_state* cur_ch_state,
                              Channel_state* new_ch_state,
                              uint32_t freq,
                              double tempo);


/**
 * Clears a Voice state.
 * \param state   The Voice state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_clear(Voice_state* state);


#endif // K_VOICE_STATE_H


