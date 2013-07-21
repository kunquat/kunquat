

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DEVICE_STATES_H
#define K_DEVICE_STATES_H


#include <player/Device_state.h>

typedef struct Device_states Device_states;


/**
 * Creates a new Device state collection.
 *
 * \return   The new Device state collection if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_states* new_Device_states(void);


/**
 * Adds a Device state to the Device state collection.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param ds       The Device state -- must not be \c NULL and must not match
 *                 an existing state in \a states.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_add_state(Device_states* states, Device_state* state);


/**
 * Gets a Device state.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param id       The Device ID -- must be > \c 0 and must match an existing
 *                 Device state.
 *
 * \return   The Device state matching \a id.
 */
Device_state* Device_states_get_state(
        const Device_states* states,
        uint32_t id);


/**
 * Sets the audio rate.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param rate     The audio rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_set_audio_rate(Device_states* states, int32_t rate);


/**
 * Sets the audio buffer size.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param size     The new buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_set_audio_buffer_size(Device_states* states, int32_t size);


/**
 * Clears audio buffers in the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param start    The first frame to be cleared.
 * \param stop     The first frame not to be cleared -- must be less than or
 *                 equal to the buffer size.
 */
void Device_states_clear_audio_buffers(
        Device_states* states,
        uint32_t start,
        uint32_t stop);


/**
 * Resets the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 */
void Device_states_reset(Device_states* states);


/**
 * Destroys a Device state collection.
 *
 * \param dsc   The Device states, or \c NULL.
 */
void del_Device_states(Device_states* states);


#endif // K_DEVICE_STATES_H


