

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_THREAD_STATE_H
#define KQT_DEVICE_THREAD_STATE_H


#include <decl.h>
#include <init/devices/port_type.h>
#include <kunquat/limits.h>
#include <player/devices/Device_node_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Device_thread_state
{
    uint32_t device_id;
    int32_t audio_buffer_size;

    Device_node_state node_state;

    // Information on which input ports are connected to something
    // TODO: This exists only because it is currently inconvenient to find a
    //       Device node by using Device as a reference -- fix this!
    Bit_array* in_connected;

    Work_buffer* buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
};


#define DEVICE_THREAD_STATE_KEY(id) &(Device_thread_state){ .device_id = (id) }


/**
 * Create a new Device thread state.
 *
 * \param device_id           The Device ID.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   The new Device thread state if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_thread_state* new_Device_thread_state(
        uint32_t device_id, int32_t audio_buffer_size);


/**
 * Compare two Device thread states.
 *
 * \param ts1   The first state -- must not be \c NULL.
 * \param ts2   The second state -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a ts1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a ts2.
 */
int Device_thread_state_cmp(
        const Device_thread_state* ts1, const Device_thread_state* ts2);


/**
 * Set the node state of the Device state.
 *
 * \param ts           The Device thread state -- must not be \c NULL.
 * \param node_state   The node state -- must be valid.
 */
void Device_thread_state_set_node_state(
        Device_thread_state* ts, Device_node_state node_state);


/**
 * Get the node state of the Device thread state.
 *
 * \param ts   The Device thread state -- must not be \c NULL.
 *
 * \return   The node state.
 */
Device_node_state Device_thread_state_get_node_state(const Device_thread_state* ts);


/**
 * Set the audio buffer size in the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param size   The audio buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_thread_state_set_audio_buffer_size(Device_thread_state* ts, int size);


/**
 * Add an audio buffer into the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_thread_state_add_audio_buffer(
        Device_thread_state* ts, Device_port_type type, int port);


/**
 * Clear audio buffers in the Device thread state.
 *
 * \param ts          The Device state -- must not be \c NULL.
 * \param buf_start   The first frame to be cleared.
 * \param buf_stop    The first frame not to be cleared -- must be less than or
 *                    equal to the buffer size.
 */
void Device_thread_state_clear_audio_buffers(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop);


/**
 * Return an audio buffer of the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The Work buffer if one exists, otherwise \c NULL.
 */
Work_buffer* Device_thread_state_get_audio_buffer(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Return contents of an audio buffer in the Device state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The buffer contents, or \c NULL if the Work buffer does not exist.
 */
float* Device_thread_state_get_audio_buffer_contents_mut(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Mark input port as connected.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 */
void Device_thread_state_mark_input_port_connected(Device_thread_state* ts, int port);


/**
 * Check if an input port is connected.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if the \a port is connected, otherwise \c false.
 */
bool Device_thread_state_is_input_port_connected(
        const Device_thread_state* ts, int port);


/**
 * Destroy an existing Device thread state.
 *
 * \param ts   The Device thread state, or \c NULL.
 */
void del_Device_thread_state(Device_thread_state* ts);


#endif // KQT_DEVICE_THREAD_STATE_H


