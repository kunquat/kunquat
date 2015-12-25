

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


#include <Decl.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <mathnum/Tstamp.h>
#include <player/Channel_proc_state.h>
#include <player/Filter_controls.h>
#include <player/Force_controls.h>
#include <player/LFO.h>
#include <player/Pitch_controls.h>
#include <player/Slider.h>
#include <player/Time_env_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef void Voice_state_init_func(Voice_state*, const Proc_state*);


typedef int32_t Voice_state_render_voice_func(
        Voice_state*,
        Proc_state*,
        const Au_state*,
        const Work_buffers*,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


typedef void Voice_state_set_cv_bool_func(
        Voice_state*, const Device_state*, const Key_indices, bool);
typedef void Voice_state_set_cv_int_func(
        Voice_state*, const Device_state*, const Key_indices, int64_t);
typedef Linear_controls* Voice_state_get_cv_float_controls_mut_func(
        Voice_state*, const Device_state*, const Key_indices);
typedef void Voice_state_set_cv_tstamp_func(
        Voice_state*, const Device_state*, const Key_indices, const Tstamp*);


#define FILTER_ORDER (2)


typedef struct Filter_state
{
    double coeffs[FILTER_ORDER]; ///< Coefficient table.
    double mul;
    double history1[KQT_BUFFERS_MAX][FILTER_ORDER]; ///< History buffer.
    double history2[KQT_BUFFERS_MAX][FILTER_ORDER]; ///< History buffer.
} Filter_state;


struct Voice_state
{
    bool active;                   ///< Whether there is anything left to process.
    uint32_t freq;                 ///< The last mixing frequency used.
    double tempo;                  ///< The last tempo setting used.
    Channel_proc_state* cpstate;   ///< Channel-specific Processor parameters.
    Random* rand_p;                ///< Parameter random source.
    Random* rand_s;                ///< Signal random source.

    Voice_state_render_voice_func* render_voice;

    double ramp_attack;            ///< The current state of volume ramp during attack.
    double ramp_release;           ///< The current state of volume ramp during release.

    int hit_index;                 ///< The hit index (negative for normal notes).
    Pitch_controls pitch_controls;
    double orig_pitch_param;       ///< The original pitch parameter.
    double actual_pitch;           ///< The actual frequency (includes vibrato).
    double prev_actual_pitch;      ///< The actual frequency in the previous mixing cycle.

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
};


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


/**
 * Render voice signal with the Voice state.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param au_state     The Audio unit state -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of rendering -- must be >= \c 0.
 * \param buf_stop     The stop index of rendering -- must be less than or equal
 *                     to the audio buffer size.
 * \param tempo        The current tempo -- must be finite and > \c 0.
 *
 * \return   The actual stop index of rendering. This is always within
 *           the interval [\a buf_start, \a buf_stop].
 */
int32_t Voice_state_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


/**
 * Set value of a control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param value    The value to be set -- must not be \c NULL.
 */
void Voice_state_cv_generic_set(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Value* value);


/**
 * Initialise floating-point control variable in the Voice state.
 *
 * \param vstate         The Voice state -- must not be \c NULL.
 * \param dstate         The Device state -- must not be \c NULL.
 * \param key            The key of the control variable -- must not be \c NULL.
 * \param src_controls   The source Linear controls -- must not be \c NULL.
 */
void Voice_state_cv_float_init(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Linear_controls* src_controls);


/**
 * Set slide target of a floating-point control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param value    The new target value -- must be finite.
 */
void Voice_state_cv_float_slide_target(
        Voice_state* vstate, const Device_state* dstate, const char* key, double value);


/**
 * Set slide length of a floating-point control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL.
 */
void Voice_state_cv_float_slide_length(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Tstamp* length);


/**
 * Set oscillation speed of a float control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param speed    The oscillation speed -- must be finite and >= \c 0.
 */
void Voice_state_cv_float_osc_speed(
        Voice_state* vstate, const Device_state* dstate, const char* key, double speed);


/**
 * Set oscillation depth of a float control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param depth    The oscillation depth -- must be finite.
 */
void Voice_state_cv_float_osc_depth(
        Voice_state* vstate, const Device_state* dstate, const char* key, double depth);


/**
 * Set oscillation speed slide of a float control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param length   The length of the oscillation speed slide -- must not be \c NULL.
 */
void Voice_state_cv_float_osc_speed_slide(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Tstamp* length);


/**
 * Set oscillation depth slide of a float control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param length   The length of the oscillation depth slide -- must not be \c NULL.
 */
void Voice_state_cv_float_osc_depth_slide(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Tstamp* length);


#endif // K_VOICE_STATE_H


