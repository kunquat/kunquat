

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


typedef struct Device_states Device_states;


/**
 * Creates a new Device state collection.
 *
 * \return   The new Device state collection if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_states* new_Device_states(void);


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
bool Device_states_set_buffer_size(Device_states* states, int32_t size);


/**
 * Allocates internal space for the Device states.
 *
 * \param states   The Device states -- must not be \c NULL.
 * \param key      The key that caused this allocation call --
 *                 must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_states_allocate_space(Device_states* states, char* key);


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


