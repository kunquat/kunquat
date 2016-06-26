

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_STATE_H
#define KQT_PROC_STATE_H


#include <decl.h>
#include <player/devices/Device_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef void Proc_state_clear_history_func(Proc_state*);

typedef void Proc_state_set_cv_bool_func(Device_state*, const Key_indices, bool);
typedef void Proc_state_set_cv_int_func(Device_state*, const Key_indices, int64_t);
typedef void Proc_state_set_cv_float_func(Device_state*, const Key_indices, double);
typedef void Proc_state_set_cv_tstamp_func(
        Device_state*, const Key_indices, const Tstamp*);


struct Proc_state
{
    Device_state parent;

    Work_buffer* voice_buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];

    Device_state_destroy_func* destroy;
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
 * Get a voice buffer of the Processor state.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param type         The port type -- must be valid.
 * \param port         The port number -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The voice buffer if one exists, otherwise \c NULL.
 */
const Work_buffer* Proc_state_get_voice_buffer(
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
Work_buffer* Proc_state_get_voice_buffer_mut(
        Proc_state* proc_state, Device_port_type type, int port);


/**
 * Get voice buffer contents of the Processor state.
 *
 * This is a convenience function that skips the Work buffer abstraction layer.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param type         The port type -- must be valid.
 * \param port         The port number -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The Work buffer contents, or \c NULL if the buffer does not exist.
 */
const float* Proc_state_get_voice_buffer_contents(
        const Proc_state* proc_state, Device_port_type type, int port);


/**
 * Get mutable voice buffer contents of the Processor state.
 *
 * This is a convenience function that skips the Work buffer abstraction layer.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param type         The port type -- must be valid.
 * \param port         The port number -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The Work buffer contents, or \c NULL if the buffer does not exist.
 */
float* Proc_state_get_voice_buffer_contents_mut(
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


#endif // KQT_PROC_STATE_H


