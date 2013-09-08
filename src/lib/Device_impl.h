

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


#ifndef K_DEVICE_IMPL_H
#define K_DEVICE_IMPL_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <AAtree.h>
#include <Decl.h>
#include <Envelope.h>
#include <player/Device_state.h>
#include <generators/Hit_map.h>
#include <generators/Sample.h>
#include <generators/Sample_map.h>
#include <Num_list.h>
#include <Tstamp.h>


#define DEVICE_KEY_INDICES_MAX 8


/**
 * The base class of Generator and DSP implementations.
 */
struct Device_impl
{
    Device* device;
    AAtree* set_cbs;
    AAtree* update_state_cbs;

    bool (*set_audio_rate)(const Device_impl*, Device_state*, int32_t);
    bool (*set_buffer_size)(const Device_impl*, Device_state*, int32_t);
    void (*update_tempo)(const Device_impl*, Device_state*, double);
    void (*reset)(const Device_impl*, Device_state*);
    void (*destroy)(Device_impl*);
};


/**
 * Initialises the Device implementation.
 *
 * \param dimpl     The Device implementation -- must not be \c NULL.
 * \param destroy   The destructor -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_init(
        Device_impl* dimpl,
        void (*destroy)(Device_impl* dimpl));


/**
 * Registers an audio rate set function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param set     The audio rate set function, or \c NULL.
 */
void Device_impl_register_set_audio_rate(
        Device_impl* dimpl,
        bool (*set)(const Device_impl*, Device_state*, int32_t));


/**
 * Registers a buffer size set function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param set     The buffer size set function, or \c NULL.
 */
void Device_impl_register_set_buffer_size(
        Device_impl* dimpl,
        bool (*set)(const Device_impl*, Device_state*, int32_t));


/**
 * Registers a tempo update function.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param update   The tempo update function, or \c NULL.
 */
void Device_impl_register_update_tempo(
        Device_impl* dimpl,
        void (*update)(const Device_impl*, Device_state*, double));


/**
 * Registers a Device state reset function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param reset   The reset function -- must not be \c NULL.
 */
void Device_impl_register_reset_device_state(
        Device_impl* dimpl,
        void (*reset)(const Device_impl*, Device_state*));


/**
 * Registers a boolean value set function.
 *
 * The key pattern may contain \c 0 to \c DEVICE_KEY_INDICES_MAX sequences of
 * XX* which are matched against hexadecimal numbers of actual keys. Example:
 *
 *    voice_XX/param_XXX/p_volume.jsonf
 *
 * Any indices with the correct amount of hexadecimal digits (here, 2 and 3)
 * are extracted from the actual updated key and passed as an array of
 * integers to the callback functions.
 *
 * An XX* sequence must be followed by a forward slash '/'.
 *
 * WARNING: A literal part of your key must not contain a valid hexadecimal
 * number followed by a forward slash '/', Example:
 *
 *    echo_eff/tap_XX/p_volume.jsonf <- Invalid: eff is interpreted as hex
 *    echo_XXX/tap_XX/p_volume.jsonf <- The above would get confused with this
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_bool(
        Device_impl* dimpl,
        const char* keyp,
        bool default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            bool));


/**
 * Registers a float value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_float(
        Device_impl* dimpl,
        const char* keyp,
        double default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            double));


/**
 * Registers an integer value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_int(
        Device_impl* dimpl,
        const char* keyp,
        int64_t default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            int64_t));


/**
 * Registers a timestamp value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        const Tstamp* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Tstamp*));


/**
 * Registers an envelope value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_envelope(
        Device_impl* dimpl,
        const char* keyp,
        const Envelope* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Envelope*));


/**
 * Registers a sample value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_sample(
        Device_impl* dimpl,
        const char* keyp,
        const Sample* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Sample*));


/**
 * Registers a sample parameters value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_sample_params(
        Device_impl* dimpl,
        const char* keyp,
        const Sample_params* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Sample_params*));


/**
 * Registers a sample map value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_sample_map(
        Device_impl* dimpl,
        const char* keyp,
        const Sample_map* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Sample_map*));


/**
 * Registers a hit map value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_hit_map(
        Device_impl* dimpl,
        const char* keyp,
        const Hit_map* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Hit_map*));


/**
 * Registers a number list value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param keyp          The key pattern -- must not be \c NULL.
 * \param default_val   The default value passed to callbacks
 *                      -- must not be \c NULL.
 * \param set_func      The set function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_num_list(
        Device_impl* dimpl,
        const char* keyp,
        const Num_list* default_val,
        bool (*set_func)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Num_list*));


/**
 * Registers a boolean value state update function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param update_state   The Device state update callback function
 *                       -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_state_bool(
        Device_impl* dimpl,
        const char* keyp,
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            bool));


/**
 * Registers a float value state update function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param update_state   The Device state update callback function
 *                       -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_state_float(
        Device_impl* dimpl,
        const char* keyp,
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            double));


/**
 * Registers a integer value state update function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param update_state   The Device state update callback function
 *                       -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_state_int(
        Device_impl* dimpl,
        const char* keyp,
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            int64_t));


/**
 * Registers a timestamp value state update function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param update_state   The Device state update callback function
 *                       -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_state_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Tstamp*));


/**
 * Resets a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 */
void Device_impl_reset_device_state(
        const Device_impl* dimpl,
        Device_state* dstate);


/**
 * Sets the audio rate of a Device state.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param dstate       The Device state -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate);


/**
 * Sets the buffer size of a Device state.
 *
 * \param dimpl         The Device implementation -- must not be \c NULL.
 * \param dstate        The Device state -- must not be \c NULL.
 * \param buffer_size   The buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_set_buffer_size(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t buffer_size);


/**
 * Updates the tempo of a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param tempo    The new tempo -- must be finite and > \c 0.
 */
void Device_impl_update_tempo(
        const Device_impl* dimpl,
        Device_state* dstate,
        double tempo);


/**
 * Sets a key in the Device implementation.
 *
 * The actual data is retrieved from device parameters.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param key     The key -- must be valid.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_set_key(Device_impl* dimpl, const char* key);


/**
 * Notifies a Device state of a Device implementation key change.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param key      The key -- must be valid.
 * \param dstate   The Device state -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_notify_key_change(
        const Device_impl* dimpl,
        const char* key,
        Device_state* dstate);


/**
 * Updates a boolean value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_update_state_bool(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        bool value);


/**
 * Updates a float value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_update_state_float(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        double value);


/**
 * Updates a integral value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_update_state_int(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        int64_t value);


/**
 * Updates a timestamp value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_update_state_tstamp(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* value);


/**
 * Deinitialises the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 */
void Device_impl_deinit(Device_impl* dimpl);


/**
 * Destroys the Device implementation.
 *
 * \param dimpl   The Device implementation, or \c NULL.
 */
void del_Device_impl(Device_impl* dimpl);


#endif // K_DEVICE_IMPL_H


