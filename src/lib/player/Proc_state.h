

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


#include <stdbool.h>
#include <stdlib.h>

#include <Audio_buffer.h>
#include <containers/Bit_array.h>
#include <player/Device_state.h>


typedef struct Proc_state
{
    Device_state parent;

    Audio_buffer* voice_buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
    Bit_array* voice_out_buffers_modified;
} Proc_state;


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
 * Reset the Processor state.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 */
void Proc_state_reset(Proc_state* proc_state);


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
 * Deinitialises the Processor state.
 *
 * \param dstate   The Processor state -- must not be \c NULL.
 */
void Proc_state_deinit(Device_state* dstate);


#endif // K_PROC_STATE_H


