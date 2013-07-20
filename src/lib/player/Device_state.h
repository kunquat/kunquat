

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


#ifndef K_DEVICE_STATE_H
#define K_DEVICE_STATE_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <Audio_buffer.h>
#include <Device.h>


/**
 * Transient state of a Device.
 */
typedef struct Device_state
{
    uint32_t device_id;
    const Device* device;

    int32_t audio_rate;
    int32_t audio_buffer_size;

    Audio_buffer* buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];

    // TODO: virtual functions
} Device_state;


#define DEVICE_STATE_KEY(id) &(Device_state){ .device_id = (id) }


/**
 * Initialises the Device state.
 *
 * \param ds                  The Device state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 */
void Device_state_init(
        Device_state* ds,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


/**
 * Compares two Device states.
 *
 * \param ds1   The first state -- must not be \c NULL.
 * \param ds2   The second state -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a ds1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a ds2.
 */
int Device_state_cmp(const Device_state* ds1, const Device_state* ds2);


/**
 * Sets the audio rate.
 *
 * \param ds     The Device state -- must not be \c NULL.
 * \param rate   The audio rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_set_audio_rate(Device_state* ds, int32_t rate);


/**
 * Sets the audio buffer size.
 *
 * \param ds     The Device state -- must not be \c NULL.
 * \param size   The new buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_set_audio_buffer_size(Device_state* ds, int32_t size);


/**
 * Allocates internal space for the Device state.
 *
 * \param ds    The Device state -- must not be \c NULL.
 * \param key   The key that caused this allocation call --
 *              must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_allocate_space(Device_state* ds, char* key);


/**
 * Clears audio buffers in the Device state.
 *
 * \param ds      The Device state -- must not be \c NULL.
 * \param start   The first frame to be cleared.
 * \param stop    The first frame not to be cleared -- must be less than or
 *                equal to the buffer size.
 */
void Device_state_clear_audio_buffers(
        Device_state* ds,
        uint32_t start,
        uint32_t stop);


/**
 * Resets the Device state.
 *
 * \param ds   The Device state -- must not be \c NULL.
 */
void Device_state_reset(Device_state* ds);


/**
 * Destroys the Device state.
 *
 * \param ds   The Device state, or \c NULL.
 */
void del_Device_state(Device_state* ds);


#endif // K_DEVICE_STATE_H


