

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


#ifndef KQT_DEVICE_STATE_H
#define KQT_DEVICE_STATE_H


#include <decl.h>
#include <init/devices/port_type.h>
#include <kunquat/limits.h>
#include <player/devices/Device_node_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef Device_state* Device_state_create_func(
        const Device*, int32_t audio_rate, int32_t buffer_size);

typedef bool Device_state_set_audio_rate_func(Device_state*, int32_t audio_rate);
typedef bool Device_state_set_audio_buffer_size_func(Device_state*, int32_t buffer_size);
typedef void Device_state_set_tempo_func(Device_state*, double tempo);
typedef void Device_state_reset_func(Device_state*);

typedef void Device_state_render_mixed_func(
        Device_state*,
        Device_thread_state*,
        const Work_buffers*,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);

typedef void Device_state_destroy_func(Device_state*);


/**
 * Transient state of a Device.
 */
struct Device_state
{
    uint32_t device_id;
    const Device* device;

    int32_t audio_rate;
    int32_t audio_buffer_size;

    // Protected interface
    bool (*add_buffer)(struct Device_state*, Device_port_type, int port);
    Device_state_set_audio_rate_func* set_audio_rate;
    Device_state_set_audio_buffer_size_func* set_audio_buffer_size;
    Device_state_set_tempo_func* set_tempo;
    Device_state_reset_func* reset;
    Device_state_render_mixed_func* render_mixed;
    Device_state_destroy_func* destroy;
};


#define DEVICE_STATE_KEY(id) &(Device_state){ .device_id = (id) }


/**
 * Initialise the Device state.
 *
 * \param ds                  The Device state -- must not be \c NULL.
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_init(
        Device_state* ds,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);


/**
 * Create a plain Device state.
 *
 * Most Devices should provide their own constructors and use
 * \a Device_state_init instead.
 *
 * \param device              The Device -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   The Device state if successful, or \c NULL if memory allocation
 *           failed.
 */
Device_state* new_Device_state_plain(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);


/**
 * Get the Device associated with the Device state.
 *
 * \param ds   The Device state -- must not be \c NULL.
 *
 * \return   The Device.
 */
const Device* Device_state_get_device(const Device_state* ds);


/**
 * Set the audio rate.
 *
 * \param ds           The Device state -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_set_audio_rate(Device_state* ds, int32_t audio_rate);


/**
 * Get the audio rate.
 *
 * \param ds   The Device state -- must not be \c NULL.
 *
 * \return   The audio rate.
 */
int32_t Device_state_get_audio_rate(const Device_state* ds);


/**
 * Set the audio buffer size.
 *
 * \param ds     The Device state -- must not be \c NULL.
 * \param size   The new buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_set_audio_buffer_size(Device_state* ds, int32_t size);


/**
 * Add an audio buffer into the Device state.
 *
 * \param ds     The Device state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and
 *               < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_state_add_audio_buffer(Device_state* ds, Device_port_type type, int port);


/**
 * Set the tempo of the Device state.
 *
 * \param ds      The Device state -- must not be \c NULL.
 * \param tempo   The new tempo -- must be finite and > \c 0.
 */
void Device_state_set_tempo(Device_state* ds, double tempo);


/**
 * Reset the Device state.
 *
 * \param ds   The Device state -- must not be \c NULL.
 */
void Device_state_reset(Device_state* ds);


/**
 * Render mixed signal in the Device state.
 *
 * \param ds          The Device state -- must not be \c NULL.
 * \param ts          The Device thread state -- must not be \c NULL.
 * \param wbs         The Work buffers -- must not be \c NULL.
 * \param buf_start   The start index of rendering -- must be >= \c 0.
 * \param buf_stop    The stop index of rendering -- must be less than or equal
 *                    to the audio buffer size.
 * \param tempo       The current tempo -- must be finite and > \c 0.
 */
void Device_state_render_mixed(
        Device_state* ds,
        Device_thread_state* ts,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


/**
 * Deinitialise the Device state.
 *
 * This function frees all the dynamically allocated resources within the base
 * Device state structure, but not the stucture itself.
 *
 * \param ds   The Device state -- must not be \c NULL.
 */
void Device_state_deinit(Device_state* ds);


/**
 * Destroy the Device state.
 *
 * \param ds   The Device state, or \c NULL.
 */
void del_Device_state(Device_state* ds);


#endif // KQT_DEVICE_STATE_H


