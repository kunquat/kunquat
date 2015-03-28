

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DEVICE_H
#define K_DEVICE_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <Audio_buffer.h>
#include <Decl.h>
#include <devices/Device_params.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <player/Device_states.h>
#include <string/Streader.h>
#include <Tstamp.h>


typedef void Device_process_signal_func(
        const Device*,
        Device_states*,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo);


struct Device
{
    uint32_t id;
    bool existent;
    bool req_impl;

    bool enable_signal_support;

    Device_params* dparams;
    Device_impl* dimpl;

    Device_state* (*create_state)(const struct Device*, int32_t, int32_t);
    bool (*set_audio_rate)(const struct Device*, Device_states*, int32_t);
    bool (*set_buffer_size)(const struct Device*, Device_states*, int32_t);
    void (*update_tempo)(const struct Device*, Device_states*, double);
    void (*reset)(const struct Device*, Device_states*);
    Device_process_signal_func* process_signal;

    bool existence[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
};


/**
 * Initialise the Device.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param req_impl   \c true if the Device requires a Device implementation,
 *                   otherwise \c false.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init(Device* device, bool req_impl);


/**
 * Return the ID of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The ID of the Device.
 */
uint32_t Device_get_id(const Device* device);


/**
 * Find out if the Device has a complete type.
 *
 * All audio units have a complete type; a processor has a complete type if it
 * has a Device implementation.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if the Device has a complete type, otherwise \c false.
 */
bool Device_has_complete_type(const Device* device);


/**
 * Set the existent status of the Device.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param existent   The existent flag.
 */
void Device_set_existent(Device* device, bool existent);


/**
 * Get the existent status of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if the Device is existent, otherwise \c false.
 */
bool Device_is_existent(const Device* device);


/**
 * Set a Device implementation of the Device.
 *
 * A previously set Device implementation will be destroyed.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param dimpl    The Device implementation, or \c NULL.
 *
 * \return   \c true if successful, or \c false if initialisation of Device
 *           implementation failed.
 */
bool Device_set_impl(Device* device, Device_impl* dimpl);


/**
 * Create a new Device state for the Device.
 *
 * \param device        The Device -- must not be \c NULL.
 * \param audio_rate    The audio rate -- must be > \c 0.
 * \param buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   The new Device state if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_state* Device_create_state(
        const Device* device, int32_t audio_rate, int32_t buffer_size);


/**
 * Set a state creator for the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param creator   The creator function, or \c NULL for default creator.
 */
void Device_set_state_creator(
        Device* device, Device_state* (*creator)(const Device*, int32_t, int32_t));


/**
 * Register the audio rate set function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param set      The audio rate set function -- must not be \c NULL.
 */
void Device_register_set_audio_rate(
        Device* device, bool (*set)(const Device*, Device_states*, int32_t));


/**
 * Register the audio buffer size set function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param set      The audio buffer size set function -- must not be \c NULL.
 */
void Device_register_set_buffer_size(
        Device* device, bool (*set)(const Device*, Device_states*, int32_t));


/**
 * Register the tempo update function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param update   The tempo update function -- must not be \c NULL.
 */
void Device_register_update_tempo(
        Device* device, void (*update)(const Device*, Device_states*, double));


/**
 * Set the playback reset function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param reset    The reset function -- must not be \c NULL.
 */
void Device_set_reset(Device* device, void (*reset)(const Device*, Device_states*));


/**
 * Set signal processing support.
 *
 * Note that signal processing may be always disabled for certain Devices.
 *
 * \param proc      The Device -- must not be \c NULL.
 * \param enabled   \c true if signal processing should be enabled, otherwise
 *                  \c false.
 */
void Device_set_signal_support(Device* device, bool enabled);


/**
 * Set the signal process function of the Device.
 *
 * \param device           The Device -- must not be \c NULL.
 * \param process_signal   The signal process function, or \c NULL.
 */
void Device_set_process(Device* device, Device_process_signal_func* process_signal);


/**
 * Set existence of a port in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 * \param exists   \c true if the port exists, otherwise \c false.
 */
void Device_set_port_existence(
        Device* device, Device_port_type type, int port, bool exists);


/**
 * Get existence of a port in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if the port exists, otherwise \c false.
 */
bool Device_get_port_existence(const Device* device, Device_port_type type, int port);


/**
 * Set the audio rate of Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param rate      The mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_audio_rate(const Device* device, Device_states* dstates, int32_t rate);


/**
 * Update the tempo of the Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param tempo     The new tempo -- must be finite and > \c 0.
 */
void Device_update_tempo(const Device* device, Device_states* dstates, double tempo);


/**
 * Resize the buffers of Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param size      The new buffer size -- must be > \c 0 and <=
 *                  \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_buffer_size(const Device* device, Device_states* dstates, int32_t size);


/**
 * Reset the internal playback state of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
void Device_reset(const Device* device, Device_states* dstates);


/**
 * Synchronise the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_sync(Device* device);


/**
 * Synchronise the Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
bool Device_sync_states(const Device* device, Device_states* dstates);


/**
 * Set a key in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param key      The key that changed -- must not be \c NULL.
 * \param sr       The Streader of the data -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_set_key(Device* device, const char* key, Streader* sr);


/**
 * Notify a Device state of a Device key change.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device state collection -- must not be \c NULL.
 * \param key       The key -- must be valid.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_set_state_key(const Device* device, Device_states* dstates, const char* key);


/**
 * Notify the Device state of a key change and updates the internal state.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param key       The key that changed -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_update_state_key(
        const Device* device, Device_states* dstates, const char* key);


/**
 * Process audio in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 * \param start    The first frame to be processed -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be processed -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be cleared.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Device_process(
        const Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


/**
 * Print a textual description of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param out      The output file -- must not be \c NULL.
 */
void Device_print(const Device* device, FILE* out);


/**
 * Deinitialise the Device.
 *
 * \param device   The Device, or \c NULL.
 */
void Device_deinit(Device* device);


#endif // K_DEVICE_H


