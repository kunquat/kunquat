

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_H
#define KQT_DEVICE_H


#include <decl.h>
#include <init/devices/Device_params.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <mathnum/Tstamp.h>
#include <player/Device_states.h>
#include <player/devices/Device_state.h>
#include <player/Linear_controls.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


typedef enum
{
    DEVICE_CONTROL_VAR_MODE_MIXED,
    DEVICE_CONTROL_VAR_MODE_VOICE,
} Device_control_var_mode;


typedef void Device_set_control_var_generic_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Random*,
        Channel*,
        const char* var_name,
        const Value* value);

typedef void Device_init_control_vars_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Random*,
        Channel*);


struct Device
{
    uint32_t id;
    bool existent;
    bool req_impl;

    bool enable_signal_support;

    Device_params* dparams;
    Device_impl* dimpl;

    Device_state_create_func* create_state;

    Device_set_control_var_generic_func* set_control_var_generic;
    Device_init_control_vars_func* init_control_vars;

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
 */
void Device_set_impl(Device* device, Device_impl* dimpl);


/**
 * Get the Device implementation of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The Device implementation of the Device, or \c NULL if one does
 *           not exist.
 */
const Device_impl* Device_get_impl(const Device* device);


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
        Device* device, Device_state_create_func* creator);


/**
 * Set mixed signal processing support.
 *
 * Note that mixed signals may be always disabled for certain Devices.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param enabled   \c true if mixed signals should be enabled, otherwise
 *                  \c false.
 */
void Device_set_mixed_signals(Device* device, bool enabled);


/**
 * Get mixed signal support information.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if \a device has mixed signals enabled, otherwise \c false.
 */
bool Device_get_mixed_signals(const Device* device);


/**
 * Set function for setting control variables.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param set_func   The generic Value setting function -- must not be \c NULL.
 */
void Device_register_set_control_var_generic(
        Device* device, Device_set_control_var_generic_func* set_func);


/**
 * Set function for initialising control variables.
 *
 * \param device      The Device -- must not be \c NULL.
 * \param init_func   The initialisation function -- must not be \c NULL.
 */
void Device_register_init_control_vars(
        Device* device, Device_init_control_vars_func* init_func);


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
 * This function is called after the corresponding call of \a Device_set_key.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device state collection -- must not be \c NULL.
 * \param key       The key -- must be valid.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_set_state_key(const Device* device, Device_states* dstates, const char* key);


/**
 * Set new value of a Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param random     The Random source -- must not be \c NULL.
 * \param channel    The Channel -- must not be \c NULL if
 *                   \a mode == \c DEVICE_CONTROL_VAR_MODE_VOICE.
 * \param var_name   The name of the control variable -- must not be \c NULL.
 * \param value      The new value -- must not be \c NULL and must have a
 *                   realtime type.
 */
void Device_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value);


/**
 * Initialise all control variables of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param mode      The Device control variable mode.
 * \param random    Global Random source -- must not be \c NULL if
 *                  \a mode == \c DEVICE_CONTROL_VAR_MODE_MIXED.
 * \param channel   The Channel -- must not be \c NULL if
 *                  \a mode == \c DEVICE_CONTROL_VAR_MODE_VOICE.
 */
void Device_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel);


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


#endif // KQT_DEVICE_H


