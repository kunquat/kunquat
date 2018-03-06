

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_VOICE_STATE_H
#define KQT_VOICE_STATE_H


#include <decl.h>
#include <init/devices/Proc_type.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <mathnum/Tstamp.h>
#include <player/Force_controls.h>
#include <player/LFO.h>
#include <player/Pitch_controls.h>
#include <player/Slider.h>
#include <player/Time_env_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef int32_t Voice_state_get_size_func(void);


typedef void Voice_state_init_func(Voice_state*, const Proc_state*);


typedef int32_t Voice_state_render_voice_func(
        Voice_state*,
        Proc_state*,
        const Device_thread_state*,
        const Au_state*,
        const Work_buffers*,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


typedef void Voice_state_set_cv_bool_func(
        Voice_state*, const Device_state*, const Key_indices, bool);
typedef void Voice_state_set_cv_int_func(
        Voice_state*, const Device_state*, const Key_indices, int64_t);
typedef void Voice_state_set_cv_float_func(
        Voice_state*, const Device_state*, const Key_indices, double);
typedef void Voice_state_set_cv_tstamp_func(
        Voice_state*, const Device_state*, const Key_indices, const Tstamp*);


typedef void Voice_state_fire_event_func(
        Voice_state*, const Device_state*, const char*, const Value*);


struct Voice_state
{
    Proc_type proc_type;

    bool active;                   ///< Whether there is anything left to process.
    int32_t keep_alive_stop;

    Random* rand_p;                ///< Parameter random source.
    Random* rand_s;                ///< Signal random source.
    Work_buffer* wb;

    bool expr_filters_applied;
    char ch_expr_name[KQT_VAR_NAME_MAX];
    char note_expr_name[KQT_VAR_NAME_MAX];

    char test_proc_param[KQT_VAR_NAME_MAX];

    double ramp_attack;            ///< The current state of volume ramp during attack.

    int hit_index;                 ///< The hit index (negative for normal notes).

    int64_t pos;                   ///< The current playback position.
    double pos_rem;                ///< The current playback position remainder.
    int64_t rel_pos;               ///< The current relative playback position.
    double rel_pos_rem;            ///< The current relative playback position remainder.
    double dir;                    ///< The current playback direction.
    bool note_on;                  ///< Whether the note is still on.
    int64_t noff_pos;              ///< Note Off position.
    double noff_pos_rem;           ///< Note Off position remainder.
};


/**
 * Initialise a Voice state.
 *
 * \param state       The Voice state -- must not be \c NULL.
 * \param proc_type   The Processor type -- must be valid.
 * \param rand_p      The parameter Random source -- must not be \c NULL.
 * \param rand_s      The signal Random source -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_init(
        Voice_state* state, Proc_type proc_type, Random* rand_p, Random* rand_s);


/**
 * Set the Work buffer associated with the Voice state.
 *
 * \param state    The Voice state -- must not be \c NULL.
 * \param wb       The Work buffer, or \c NULL.
 */
void Voice_state_set_work_buffer(Voice_state* state, Work_buffer* wb);


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
 * \param vstate       The Voice state, or \c NULL if the associated processor
 *                     has a stateless voice processing function.
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param proc_ts      The Device thread state -- must not be \c NULL.
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
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


/**
 * Set request to keep the Voice state alive.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param stop     The buffer index at which it is OK to stop processing
 *                 -- must be >= \c 0. Note that the Voice state may be kept
 *                 alive longer and the associated Processor must provide data
 *                 as long as the output may contain non-zero values.
 */
void Voice_state_set_keep_alive_stop(Voice_state* vstate, int32_t stop);


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


#endif // KQT_VOICE_STATE_H


