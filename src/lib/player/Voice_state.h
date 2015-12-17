

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


#ifndef K_VOICE_STATE_H
#define K_VOICE_STATE_H


#include <stdbool.h>
#include <stdint.h>

#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <pitch_t.h>
#include <player/Channel_proc_state.h>
#include <player/Filter_controls.h>
#include <player/Force_controls.h>
#include <player/LFO.h>
#include <player/Pitch_controls.h>
#include <player/Slider.h>
#include <player/Time_env_state.h>
#include <Tstamp.h>


#define FILTER_ORDER (2)


typedef struct Filter_state
{
    double coeffs[FILTER_ORDER]; ///< Coefficient table.
    double mul;
    double history1[KQT_BUFFERS_MAX][FILTER_ORDER]; ///< History buffer.
    double history2[KQT_BUFFERS_MAX][FILTER_ORDER]; ///< History buffer.
} Filter_state;


typedef struct Voice_state
{
    bool active;                   ///< Whether there is anything left to process.
    uint32_t freq;                 ///< The last mixing frequency used.
    double tempo;                  ///< The last tempo setting used.
    Channel_proc_state* cpstate;   ///< Channel-specific Processor parameters.
    Random* rand_p;                ///< Parameter random source.
    Random* rand_s;                ///< Signal random source.

    double ramp_attack;            ///< The current state of volume ramp during attack.
    double ramp_release;           ///< The current state of volume ramp during release.

    int hit_index;                 ///< The hit index (negative for normal notes).
    Pitch_controls pitch_controls;
    double orig_pitch_param;       ///< The original pitch parameter.
    pitch_t actual_pitch;          ///< The actual frequency (includes vibrato).
    pitch_t prev_actual_pitch;     ///< The actual frequency in the previous mixing cycle.

    bool arpeggio;                 ///< Arpeggio enabled.
    double arpeggio_ref;           ///< Arpeggio reference note in cents.
    double arpeggio_length;        ///< Length of one note in the arpeggio.
    double arpeggio_frames;        ///< Frames left of the current note in the arpeggio.
    int arpeggio_note;             ///< Current note in the arpeggio.
    double arpeggio_tones[KQT_ARPEGGIO_NOTES_MAX]; ///< Tones in the arpeggio.

    uint64_t pos;                  ///< The current playback position.
    double pos_rem;                ///< The current playback position remainder.
    uint64_t rel_pos;              ///< The current relative playback position.
    double rel_pos_rem;            ///< The current relative playback position remainder.
    double dir;                    ///< The current playback direction.
    bool note_on;                  ///< Whether the note is still on.
    uint64_t noff_pos;             ///< Note Off position.
    double noff_pos_rem;           ///< Note Off position remainder.

    Time_env_state force_env_state;
    Time_env_state force_rel_env_state;

    Force_controls force_controls;
    double actual_force;           ///< The current actual force (includes tremolo & envs).

    double panning;                ///< The current panning.
    double actual_panning;         ///< The current actual panning.
    Slider panning_slider;

    float pitch_pan_ref_param;     ///< Pitch value that maps to the stored pitch-pan value.
    float pitch_pan_value;

    Time_env_state env_filter_state;
    Time_env_state env_filter_rel_state;

    Filter_controls filter_controls;
    double actual_lowpass;         ///< The current actual lowpass parameter.

    // Lowpass filter implementation state
    double applied_lowpass;
    double applied_resonance;
    double true_lowpass;
    double true_resonance;
    double lowpass_xfade_pos;
    double lowpass_xfade_update;
    int lowpass_xfade_state_used;
    int lowpass_state_used;
    Filter_state lowpass_state[2];
} Voice_state;


/**
 * Initialise a Voice state.
 *
 * \param state     The Voice state -- must not be \c NULL.
 * \param cpstate   The Channel-specific Processor state -- must not be
 *                  \c NULL.
 * \param rand_p    The parameter Random source -- must not be \c NULL.
 * \param rand_s    The signal Random source -- must not be \c NULL.
 * \param freq      The mixing frequency -- must be > \c 0.
 * \param tempo     The current tempo -- must be > \c 0.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_init(
        Voice_state* state,
        Channel_proc_state* cpstate,
        Random* rand_p,
        Random* rand_s,
        uint32_t freq,
        double tempo);


/**
 * Clear a Voice state.
 *
 * \param state   The Voice state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_clear(Voice_state* state);


#endif // K_VOICE_STATE_H


