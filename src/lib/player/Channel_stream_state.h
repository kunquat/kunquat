

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_CHANNEL_STREAM_STATE_H
#define KQT_CHANNEL_STREAM_STATE_H


#include <decl.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * A dictionary for stream states in a Channel.
 */
typedef struct Channel_stream_state Channel_stream_state;


/**
 * Create a new Channel stream state.
 *
 * \return   The new Channel stream state if successful, or \c NULL if memory
 *           allocation failed.
 */
Channel_stream_state* new_Channel_stream_state(void);


/**
 * Set the audio rate of the Channel stream state.
 *
 * \param state        The Channel stream state -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Channel_stream_state_set_audio_rate(
        Channel_stream_state* state, int32_t audio_rate);


/**
 * Set the tempo of the Channel stream state.
 *
 * \param state   The Channel stream state -- must not be \c NULL.
 * \param tempo   The tempo -- must be positive.
 */
void Channel_stream_state_set_tempo(Channel_stream_state* state, double tempo);


/**
 * Add a stream to the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Channel_stream_state_add_entry(
        Channel_stream_state* state, const char* stream_name);


/**
 * Set a value of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param value         The value -- must be finite.
 *
 * \return   \c true if the new value was actually set, or \c false if \a state
 *           does not contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_value(
        Channel_stream_state* state, const char* stream_name, double value);


/**
 * Set sliding target of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param value         The target value -- must be finite.
 *
 * \return   \c true if \a value was applied, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_slide_target(
        Channel_stream_state* state, const char* stream_name, double value);


/**
 * Set slide length of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param length        The new slide length -- must not be \c NULL.
 *
 * \return   \c true if \a length was set, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_slide_length(
        Channel_stream_state* state, const char* stream_name, const Tstamp* length);


/**
 * Set oscillation speed of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param speed         The new speed -- must be finite and >= \c 0.
 *
 * \return   \c true if \a speed was applied, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_osc_speed(
        Channel_stream_state* state, const char* stream_name, double speed);


/**
 * Set oscillation depth of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param depth         The new depth -- must be finite.
 *
 * \return   \c true if \a depth was applied, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_osc_depth(
        Channel_stream_state* state, const char* stream_name, double depth);


/**
 * Set oscillation speed slide of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param length        The slide length -- must not be \c NULL.
 *
 * \return   \c true if \a length was set, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_osc_speed_slide(
        Channel_stream_state* state, const char* stream_name, const Tstamp* length);


/**
 * Set oscillation depth slide of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param length        The slide length -- must not be \c NULL.
 *
 * \return   \c true if \a length was set, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_osc_depth_slide(
        Channel_stream_state* state, const char* stream_name, const Tstamp* length);


/**
 * Set Linear controls of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param controls      The Linear controls -- must not be \c NULL.
 *
 * \return   \c true if \a controls were set, or \c false if \a state does not
 *           contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_controls(
        Channel_stream_state* state,
        const char* stream_name,
        const Linear_controls* controls);


/**
 * Get Linear controls of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 *
 * \return   The Linear controls if a corresponding stream exists, otherwise \c NULL.
 */
const Linear_controls* Channel_stream_state_get_controls(
        const Channel_stream_state* state, const char* stream_name);


/**
 * Set carrying state of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param enabled       \c true to enable carrying, \c false to disable.
 *
 * \return   \c true if the new carrying state was set, or \c false if \a state
 *           does not contain a stream called \a stream_name.
 */
bool Channel_stream_state_set_carrying_enabled(
        Channel_stream_state* state, const char* stream_name, bool enabled);


/**
 * Get carrying state of a stream in the Channel stream state.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 *
 * \return   \c true if carrying is enabled for the stream called
 *           \a stream_name, otherwise \c false.
 */
bool Channel_stream_state_is_carrying_enabled(
        const Channel_stream_state* state, const char* stream_name);


/**
 * Apply overriden settings in the Channel stream state to Linear controls.
 *
 * \param state         The Channel stream state -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid variable name.
 * \param controls      The Linear controls -- must not be \c NULL.
 *
 * \return   \c true if settings were applied, or \c false if \a stream_name
 *           was not found in \a state.
 */
bool Channel_stream_state_apply_overrides(
        const Channel_stream_state* state,
        const char* stream_name,
        Linear_controls* controls);


/**
 * Update streams in the Channel stream state.
 *
 * \param state        The Channel stream state -- must not be \c NULL.
 * \param step_count   Number of steps to update -- must be >= \c 0.
 */
void Channel_stream_state_update(Channel_stream_state* state, int64_t step_count);


/**
 * Reset all streams in the Channel stream state.
 *
 * \param state   The Channel stream state -- must not be \c NULL.
 */
void Channel_stream_state_reset(Channel_stream_state* state);


/**
 * Destroy an existing Channel stream state.
 *
 * \param state   The Channel stream state, or \c NULL.
 */
void del_Channel_stream_state(Channel_stream_state* state);


#endif // KQT_CHANNEL_STREAM_STATE_H


