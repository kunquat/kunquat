

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <Device_params.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <player/Device_states.h>
#include <Tstamp.h>


struct Device
{
    uint32_t id;
    bool existent;
    uint32_t mix_rate;
    uint32_t buffer_size;

    Device_params* dparams;
    Device_impl* dimpl;

    Device_state* (*create_state)(const struct Device*, int32_t, int32_t);
    bool (*set_mix_rate)(struct Device*, Device_states*, uint32_t);
    void (*update_tempo)(const struct Device*, Device_states*, double);
    bool (*set_buffer_size)(struct Device*, Device_states*, uint32_t);
    void (*reset)(struct Device*, Device_states*);
    //bool (*sync)(struct Device*, Device_states*);
    bool (*update_key)(struct Device*, const char*);
    void (*process)(
            struct Device*,
            Device_states*,
            uint32_t,
            uint32_t,
            uint32_t,
            double);

    bool reg[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
};


/**
 * Initialises the Device.
 *
 * \param device        The Device -- must not be \c NULL.
 * \param buffer_size   The current buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The current mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init(Device* device, uint32_t buffer_size, uint32_t mix_rate);


/**
 * Returns the ID of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The ID of the Device.
 */
uint32_t Device_get_id(const Device* device);


/**
 * Sets the existent status of the Device.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param existent   The existent flag.
 */
void Device_set_existent(Device* device, bool existent);


/**
 * Gets the existent status of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if the Device is existent, otherwise \c false.
 */
bool Device_is_existent(const Device* device);


/**
 * Sets a Device implementation of the Device.
 *
 * A previously set Device implementation will be destroyed.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param dimpl    The Device implementation, or \c NULL.
 *
 * \return   \c true if successful, or \c false if device sync failed.
 */
bool Device_set_impl(Device* device, Device_impl* dimpl);


/**
 * Creates a new Device state for the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The new Device state if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_state* Device_create_state(const Device* device);


/**
 * Sets a state creator for the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param creator   The creator function, or \c NULL for default creator.
 */
void Device_set_state_creator(
        Device* device,
        Device_state* (*creator)(const Device*, int32_t, int32_t));


/**
 * Sets the function for changing the mixing rate of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param changer   The change function, or \c NULL.
 */
void Device_set_mix_rate_changer(
        Device* device,
        bool (*changer)(Device*, Device_states*, uint32_t));


/**
 * Registers the tempo update function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param update   The tempo update function -- must not be \c NULL.
 */
void Device_register_update_tempo(
        Device* device,
        void (*update)(const Device*, Device_states*, double));


/**
 * Sets the function for changing the buffer size of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param changer   The change function, or \c NULL.
 */
void Device_set_buffer_size_changer(
        Device* device,
        bool (*changer)(Device*, Device_states*, uint32_t));


/**
 * Sets the playback reset function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param reset    The reset function -- must not be \c NULL.
 */
void Device_set_reset(Device* device, void (*reset)(Device*, Device_states*));


/**
 * Sets the synchronisation function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param sync     The synchronisation function -- must not be \c NULL.
 */
//void Device_set_sync(Device* device, bool (*sync)(Device*, Device_states*));


/**
 * Sets the update notification function of the Device.
 *
 * \param device       The Device -- must not be \c NULL.
 * \param update_key   The update notification function
 *                     -- must not be \c NULL.
 */
//void Device_set_update_key(
//        Device* device,
//        bool (*update_key)(Device*, const char*));


/**
 * Sets the process function of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param process   The process function -- must not be \c NULL.
 */
void Device_set_process(
        Device* device,
        void (*process)(
            Device*,
            Device_states*,
            uint32_t,
            uint32_t,
            uint32_t,
            double));


/**
 * Registers a new port for the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX. If the port is already
 *                 registered, the function does nothing.
 */
void Device_register_port(Device* device, Device_port_type type, int port);


/**
 * Unregisters a port of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX. If the port is not registered,
 *                 the function does nothing.
 */
void Device_unregister_port(Device* device, Device_port_type type, int port);


/**
 * Finds out whether a port is registered in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if the port is registered, otherwise \c false.
 */
bool Device_port_is_registered(
        const Device* device,
        Device_port_type type,
        int port);


/**
 * Sets the mixing rate of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param rate      The mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t rate);


/**
 * Updates the tempo of the Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param tempo     The new tempo -- must be finite and > \c 0.
 */
void Device_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo);


/**
 * Gets the mixing rate of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The mixing rate.
 */
uint32_t Device_get_mix_rate(const Device* device);


/**
 * Resizes the buffers in the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param size      The new buffer size -- must be > \c 0 and <=
 *                  \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_buffer_size(
        Device* device,
        Device_states* dstates,
        uint32_t size);


/**
 * Gets the buffer size of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The buffer size.
 */
uint32_t Device_get_buffer_size(const Device* device);


/**
 * Resets the internal playback state of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
void Device_reset(Device* device, Device_states* dstates);


/**
 * Synchronises the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_sync(Device* device);


/**
 * Synchronises the Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
bool Device_sync_states(Device* device, Device_states* dstates);


/**
 * Sets a key in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param key      The key that changed -- must not be \c NULL.
 * \param data     The data -- must not be \c NULL unless \a length == \c 0.
 * \param length   The length of \a data -- must be >= \c 0.
 * \param rs       The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_set_key(
        Device* device,
        const char* key,
        void* data,
        long length,
        Read_state* rs);


/**
 * Notifies a Device state of a Device key change.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param key       The key -- must be valid.
 * \param dstates   The Device state collection -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_notify_key_change(
        const Device* device,
        const char* key,
        Device_states* dstates);


/**
 * Notifies the Device state of a key change and updates the internal state.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param key       The key that changed -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_update_state_key(
        Device* device,
        Device_states* dstates,
        const char* key);


/**
 * Processes audio in the Device.
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
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


/**
 * Prints a textual description of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param out      The output file -- must not be \c NULL.
 */
void Device_print(Device* device, FILE* out);


/**
 * Deinitialises the Device.
 *
 * \param device   The Device, or \c NULL.
 */
void Device_deinit(Device* device);


#endif // K_DEVICE_H


