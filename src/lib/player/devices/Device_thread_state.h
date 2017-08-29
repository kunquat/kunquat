

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


#ifndef KQT_DEVICE_THREAD_STATE_H
#define KQT_DEVICE_THREAD_STATE_H


#include <decl.h>
#include <init/devices/port_type.h>
#include <kunquat/limits.h>
#include <player/devices/Device_node_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    DEVICE_BUFFER_MIXED = 0,
    DEVICE_BUFFER_VOICE,
    DEVICE_BUFFER_TYPES
} Device_buffer_type;


struct Device_thread_state
{
    uint32_t device_id;
    int32_t audio_buffer_size;

    Device_node_state node_state;

    bool has_mixed_audio;

    // Information on which input ports are connected to something
    // TODO: This exists only because it is currently inconvenient to find a
    //       Device node by using Device as a reference -- fix this!
    Bit_array* in_connected;

    Etable* buffers[DEVICE_BUFFER_TYPES][DEVICE_PORT_TYPES];
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
 * Add a mixed audio buffer into the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_thread_state_add_mixed_buffer(
        Device_thread_state* ts, Device_port_type type, int port);


/**
 * Clear mixed audio buffers in the Device thread state.
 *
 * \param ts          The Device state -- must not be \c NULL.
 * \param buf_start   The first frame to be cleared.
 * \param buf_stop    The first frame not to be cleared -- must be less than or
 *                    equal to the buffer size.
 */
void Device_thread_state_clear_mixed_buffers(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop);


/**
 * Return a mixed audio buffer of the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The Work buffer if one exists, otherwise \c NULL.
 */
Work_buffer* Device_thread_state_get_mixed_buffer(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Return a connected mixed audio buffer of the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The connected Work buffer if one exists, otherwise \c NULL.
 */
Work_buffer* Device_thread_state_get_connected_mixed_buffer(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Return contents of a mixed audio buffer in the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The buffer contents, or \c NULL if the Work buffer does not exist.
 */
float* Device_thread_state_get_mixed_buffer_contents_mut(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Add a voice audio buffer into the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_thread_state_add_voice_buffer(
        Device_thread_state* ts, Device_port_type type, int port);


/**
 * Clear voice audio buffers in the Device thread state.
 *
 * \param ts          The Device state -- must not be \c NULL.
 * \param buf_start   The first frame to be cleared.
 * \param buf_stop    The first frame not to be cleared -- must be less than or
 *                    equal to the buffer size.
 */
void Device_thread_state_clear_voice_buffers(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop);


/**
 * Return a voice audio buffer of the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The Work buffer if one exists, otherwise \c NULL.
 */
Work_buffer* Device_thread_state_get_voice_buffer(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Return contents of a voice audio buffer in the Device thread state.
 *
 * \param ts     The Device thread state -- must not be \c NULL.
 * \param type   The port type -- must be valid.
 * \param port   The port number -- must be >= \c 0 and < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The buffer contents, or \c NULL if the Work buffer does not exist.
 */
float* Device_thread_state_get_voice_buffer_contents(
        const Device_thread_state* ts, Device_port_type type, int port);


/**
 * Mark Device thread state as containing mixed audio.
 *
 * \param ts   The Device thread state -- must not be \c NULL.
 */
void Device_thread_state_mark_mixed_audio(Device_thread_state* ts);


/**
 * Mix rendered Voice signals to mixed signal buffers.
 *
 * \param ts          The Device thread state -- must not be \c NULL.
 * \param buf_start   The start index of mixing -- must be >= \c 0.
 * \param buf_stop    The stop index of mixing -- must be less than or equal
 *                    to the audio buffer size.
 */
void Device_thread_state_mix_voice_signals(
        Device_thread_state* ts, int32_t buf_start, int32_t buf_stop);


/**
 * Check if the Device thread state contains mixed audio.
 *
 * \param ts   The Device thread state -- must not be \c NULL.
 *
 * \return   \c true if \a ts contains mixed audio, otherwise \c false.
 */
bool Device_thread_state_has_mixed_audio(const Device_thread_state* ts);


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


