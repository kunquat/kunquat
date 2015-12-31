

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_CHANNEL_CV_STATE_H
#define K_CHANNEL_CV_STATE_H


#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <player/Linear_controls.h>
#include <Value.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * A dictionary for audio unit control variable states in a Channel.
 */
typedef struct Channel_cv_state Channel_cv_state;


/**
 * Create a new Channel control variable state.
 *
 * \return   The new Channel control variable state if successful, or \c NULL
 *           if memory allocation failed.
 */
Channel_cv_state* new_Channel_cv_state(void);


/**
 * Set the audio rate of the Channel control variable state.
 *
 * \param state        The Channel control variable state -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Channel_cv_state_set_audio_rate(Channel_cv_state* state, int32_t audio_rate);


/**
 * Set the tempo of the Channel control variable state.
 *
 * \param state   The Channel control variable state -- must not be \c NULL.
 * \param tempo   The tempo -- must be positive.
 */
void Channel_cv_state_set_tempo(Channel_cv_state* state, double tempo);


/**
 * Add a control variable entry to the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL and must be
 *                   shorter than \c KQT_VAR_NAME_MAX characters.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Channel_cv_state_add_entry(Channel_cv_state* state, const char* var_name);


/**
 * Set a value of a control variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param value      The new value -- must not be \c NULL.
 *
 * \return   \c true if the new value was actually set, or \c false if
 *           \a state does not contain an entry called \a var_name.
 */
bool Channel_cv_state_set_value(
        Channel_cv_state* state, const char* var_name, const Value* value);


/**
 * Set sliding target of a float in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param value      The target value -- must be finite.
 *
 * \return   \c true if \a value was applied, or \c false if \a state does not
 *           contain an entry called \a var_name.
 */
bool Channel_cv_state_slide_target_float(
        Channel_cv_state* state, const char* var_name, double value);


/**
 * Set slide length of a float in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param length     The new slide length -- must not be \c NULL.
 *
 * \return   \c true if \a length was set, or \c false if \a state does not
 *           contain a float entry called \a var_name.
 */
bool Channel_cv_state_slide_length_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length);


/**
 * Set oscillation speed of a float in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param speed      The new speed -- must be finite and non-negative.
 *
 * \return   \c true if \a speed was applied, or \c false if \a state does not
 *           contain a float entry called \a var_name.
 */
bool Channel_cv_state_osc_speed_float(
        Channel_cv_state* state, const char* var_name, double speed);


/**
 * Set oscillation depth of a float in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param depth      The new depth -- must be finite.
 *
 * \return   \c true if \a depth was applied, or \c false if \a state does not
 *           contain a float entry called \a var_name.
 */
bool Channel_cv_state_osc_depth_float(
        Channel_cv_state* state, const char* var_name, double depth);


/**
 * Set oscillation speed slide of a float in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param length     The slide length -- must not be \c NULL.
 *
 * \return   \c true if \a length was applied, or \c false if \a state does not
 *           contain a float entry called \a var_name.
 */
bool Channel_cv_state_osc_speed_slide_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length);


/**
 * Set oscillation depth slide of a float in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param length     The slide length -- must not be \c NULL.
 *
 * \return   \c true if \a length was applied, or \c false if \a state does not
 *           contain a float entry called \a var_name.
 */
bool Channel_cv_state_osc_depth_slide_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length);


/**
 * Get a value of a control variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 *
 * \return   The stored value if one exists, otherwise \c NULL.
 */
const Value* Channel_cv_state_get_value(
        const Channel_cv_state* state, const char* var_name);


/**
 * Get Linear controls of a float variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 *
 * \return   The Linear controls if a corresponding float variable exists,
 *           otherwise \c NULL.
 */
const Linear_controls* Channel_cv_state_get_float_controls(
        const Channel_cv_state* state, const char* var_name);


/**
 * Set carrying state of a control variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param enabled    \c true to enable carrying, \c false to disable.
 *
 * \return   \c true if the new carrying state was set, or \c false if \a state
 *           does not contain an entry with matching variable name and value type.
 */
bool Channel_cv_state_set_carrying_enabled(
        Channel_cv_state* state, const char* var_name, bool enabled);


/**
 * Get carrying state of a control variable in the Channel control variable state.
 *
 * \param state      The Channel control variable state -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 *
 * \return   \c true if carrying is enabled for the control variable called
 *           \a var_name, otherwise \c false.
 */
bool Channel_cv_state_is_carrying_enabled(
        const Channel_cv_state* state, const char* var_name);


/**
 * Update float controls in the Channel control variable state.
 *
 * \param state        The Channel control variable state -- must not be \c NULL.
 * \param step_count   Number of steps to update.
 */
void Channel_cv_state_update_float_controls(
        Channel_cv_state* state, uint64_t step_count);


/**
 * Reset all variables in the Channel control variable state.
 *
 * \param state   The Channel control variable state -- must not be \c NULL.
 */
void Channel_cv_state_reset(Channel_cv_state* state);


/**
 * Destroy an existing Channel control variable state.
 *
 * \param state   The Channel control variable state, or \c NULL.
 */
void del_Channel_cv_state(Channel_cv_state* state);


#endif // K_CHANNEL_CV_STATE_H


