

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_STATES_H
#define KQT_DEVICE_STATES_H


#include <decl.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Create a new Device state collection.
 *
 * \return   The new Device state collection if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_states* new_Device_states(void);


/**
 * Set the number of threads for space allocation in the Device states.
 *
 * \param states      The Device states -- must not be \c NULL.
 * \param new_count   The number of threads -- must be >= \c 1 and
 *                    <= \c KQT_THREADS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_set_thread_count(Device_states* states, int new_count);


/**
 * Add a Device state to the Device state collection.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param state    The Device state -- must not be \c NULL and must not match
 *                 an existing state in \a states.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_add_state(Device_states* states, Device_state* state);


/**
 * Get a Device state.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param id       The Device ID -- must be > \c 0 and must match an existing
 *                 Device state.
 *
 * \return   The Device state matching \a id.
 */
Device_state* Device_states_get_state(const Device_states* states, uint32_t id);


/**
 * Remove a Device state in the Device state collection.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param id       The Device ID -- must be > \c 0.
 */
void Device_states_remove_state(Device_states* states, uint32_t id);


/**
 * Get a Device thread state.
 *
 * \param states      The Device states -- must not be \c NULL.
 * \param thread_id   The ID of the thread accessing the Device states
 *                    -- must be a valid ID currently in use.
 * \param device_id   The Device ID -- must be > \c 0 and must match an
 *                    existing Device state.
 *
 * \return   The Device thread state.
 */
Device_thread_state* Device_states_get_thread_state(
        const Device_states* states, int thread_id, uint32_t device_id);


/**
 * Set the audio rate.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param rate     The audio rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_set_audio_rate(Device_states* states, int32_t rate);


/**
 * Set the audio buffer size.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param size     The new buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_set_audio_buffer_size(Device_states* states, int32_t size);


/**
 * Clear audio buffers in the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param start    The first frame to be cleared -- must be >= \c 0.
 * \param stop     The first frame not to be cleared -- must be less than or
 *                 equal to the buffer size.
 */
void Device_states_clear_audio_buffers(
        Device_states* states, int32_t start, int32_t stop);


/**
 * Set the tempo in the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param tempo    The new tempo -- must be finite and > \c 0.
 */
void Device_states_set_tempo(Device_states* states, double tempo);


/**
 * Prepare the Device states for mixing.
 *
 * \param dstates   The Device states -- must not be \c NULL.
 * \param conns     The Connections -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_prepare(Device_states* dstates, const Connections* conns);


/**
 * Mix buffers rendered by separate threads.
 *
 * \param dstates     The Device states -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed
 *                    -- must be less than the buffer size.
 * \param buf_stop    The stop index of the buffer area to be processed
 *                    -- must be less than or equal to the buffer size.
 */
void Device_states_mix_thread_states(
        Device_states* dstates, int32_t buf_start, int32_t buf_stop);


/**
 * Reset the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 */
void Device_states_reset(Device_states* states);


/**
 * Reset the graph search node states in the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 */
void Device_states_reset_node_states(Device_states* states);


/**
 * Destroy a Device state collection.
 *
 * \param states   The Device states, or \c NULL.
 */
void del_Device_states(Device_states* states);


#endif // KQT_DEVICE_STATES_H


