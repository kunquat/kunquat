

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_STATE_H
#define K_PROC_STATE_H


#include <containers/Bit_array.h>
#include <decl.h>
#include <player/Audio_buffer.h>
#include <player/devices/Device_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef void Proc_state_clear_history_func(Proc_state*);


typedef void Proc_state_set_cv_bool_func(Device_state*, const Key_indices, bool);
typedef void Proc_state_set_cv_int_func(Device_state*, const Key_indices, int64_t);
typedef Linear_controls* Proc_state_get_cv_float_controls_mut_func(
        Device_state*, const Key_indices);
typedef void Proc_state_set_cv_tstamp_func(
        Device_state*, const Key_indices, const Tstamp*);


struct Proc_state
{
    Device_state parent;

    Audio_buffer* voice_buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
    Bit_array* voice_out_buffers_modified;

    Device_state_set_audio_rate_func* set_audio_rate;
    Device_state_set_audio_buffer_size_func* set_audio_buffer_size;
    Device_state_set_tempo_func* set_tempo;
    Device_state_reset_func* reset;
    Device_state_render_mixed_func* render_mixed;

    Proc_state_clear_history_func* clear_history;
};


/**
 * Initialise the Processor state.
 *
 * \param proc_state          The Processor state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Proc_state_init(
        Proc_state* proc_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


/**
 * Clear Processor state history.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 */
void Proc_state_clear_history(Proc_state* proc_state);


/**
 * Clear the voice buffers of the Processor state.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 */
void Proc_state_clear_voice_buffers(Proc_state* proc_state);


/**
 * Get voice output buffer modification status.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 *
 * \return   \c true if the buffer has been modified, otherwise \c false.
 */
bool Proc_state_is_voice_out_buffer_modified(Proc_state* proc_state, int port_num);


/**
 * Get a voice buffer of the Processor state.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param type         The port type -- must be valid.
 * \param port         The port number -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The voice buffer if one exists, otherwise \c NULL.
 */
const Audio_buffer* Proc_state_get_voice_buffer(
        const Proc_state* proc_state, Device_port_type type, int port);


/**
 * Get a mutable voice buffer of the Processor state.
 *
 * For output buffers, this function marks the returned buffer as modified so
 * that any default post-processing code will be applied to the buffer later.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param type         The port type -- must be valid.
 * \param port         The port number -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The voice buffer if one exists, otherwise \c NULL.
 */
Audio_buffer* Proc_state_get_voice_buffer_mut(
        Proc_state* proc_state, Device_port_type type, int port);


/**
 * Set value of a control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param value    The value to be set -- must not be \c NULL.
 */
void Proc_state_cv_generic_set(
        Device_state* dstate, const char* key, const Value* value);


/**
 * Set slide target of a floating-point control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param value    The new target value -- must be finite.
 */
void Proc_state_cv_float_slide_target(
        Device_state* dstate, const char* key, double value);


/**
 * Set slide length of a floating-point control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL.
 */
void Proc_state_cv_float_slide_length(
        Device_state* dstate, const char* key, const Tstamp* length);


/**
 * Set oscillation speed of a float control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param speed    The oscillation speed -- must be finite and >= \c 0.
 */
void Proc_state_cv_float_osc_speed(
        Device_state* dstate, const char* key, double speed);


/**
 * Set oscillation depth of a float control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param depth    The oscillation depth -- must be finite.
 */
void Proc_state_cv_float_osc_depth(
        Device_state* dstate, const char* key, double depth);


/**
 * Set oscillation speed slide of a float control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param length   The length of the oscillation speed slide -- must not be \c NULL.
 */
void Proc_state_cv_float_osc_speed_slide(
        Device_state* dstate, const char* key, const Tstamp* length);


/**
 * Set oscillation depth slide of a float control variable in the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param length   The length of the oscillation depth slide -- must not be \c NULL.
 */
void Proc_state_cv_float_osc_depth_slide(
        Device_state* dstate, const char* key, const Tstamp* length);


/**
 * Deinitialises the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 */
void Proc_state_deinit(Device_state* dstate);


#endif // K_PROC_STATE_H


