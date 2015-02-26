

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

#include <frame.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <pitch_t.h>
#include <player/Channel_gen_state.h>
#include <player/LFO.h>
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
    Channel_gen_state* cgstate;    ///< Channel-specific Generator parameters.
    Random* rand_p;                ///< Parameter random source.
    Random* rand_s;                ///< Signal random source.

    double ramp_attack;            ///< The current state of volume ramp during attack.
    double ramp_release;           ///< The current state of volume ramp during release.
    double orig_cents;             ///< The pitch in cents used at the beginning.

    int hit_index;                 ///< The hit index (negative for normal notes).
    pitch_t pitch;                 ///< The frequency at which the note is played.
    pitch_t actual_pitch;          ///< The actual frequency (includes vibrato).
    pitch_t prev_actual_pitch;     ///< The actual frequency in the previous mixing cycle.
    Slider pitch_slider;
    LFO vibrato;

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

    double rel_fe_pos;             ///< Release force envelope position.
    int rel_fe_next_node;          ///< Next release force envelope node.
    double rel_fe_value;           ///< Current release force envelope value.
    double rel_fe_update;          ///< Release force envelope update.
    double rel_fe_scale;           ///< Current release force envelope scale factor.

    double force;                  ///< The current force (linear factor).
    double actual_force;           ///< The current actual force (includes tremolo & envs).
    Slider force_slider;
    LFO tremolo;

    double panning;                ///< The current panning.
    double actual_panning;         ///< The current actual panning.
    Slider panning_slider;

    double lowpass;                ///< The current lowpass cut-off frequency.
    double actual_lowpass;         ///< The current actual lowpass cut-off frequency.
    Slider lowpass_slider;
    LFO autowah;
    double lowpass_resonance;      ///< The filter resonance (Q factor).

    double effective_lowpass;      ///< The current filter cut-off frequency _really_ used.
    double effective_resonance;    ///< The current filter resonance _really_ used.
    bool lowpass_update;           ///< Whether filter needs to be updated.
    double lowpass_xfade_pos;      ///< Filter crossfade position.
    double lowpass_xfade_update;   ///< The update amount of the filter crossfade.
    int lowpass_xfade_state_used;  ///< State fading out during the filter crossfade.
    int lowpass_state_used;        ///< Primary filter state used.
    Filter_state lowpass_state[2]; ///< States of the filters.
} Voice_state;


/**
 * Initialise a Voice state.
 *
 * \param state     The Voice state -- must not be \c NULL.
 * \param cgstate   The Channel-specific Generator state -- must not be
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
        Channel_gen_state* cgstate,
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


