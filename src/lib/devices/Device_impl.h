

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
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

#include <containers/AAtree.h>
#include <Decl.h>
#include <devices/param_types/Envelope.h>
#include <devices/param_types/Hit_map.h>
#include <devices/param_types/Note_map.h>
#include <devices/param_types/Num_list.h>
#include <devices/param_types/Sample.h>
#include <player/Device_state.h>
#include <Tstamp.h>


#define DEVICE_KEY_INDICES_MAX 8


typedef int32_t Device_key_indices[DEVICE_KEY_INDICES_MAX];


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
 * Initialise the Device implementation.
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
 * Register an audio rate set function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param set     The audio rate set function, or \c NULL.
 */
void Device_impl_register_set_audio_rate(
        Device_impl* dimpl,
        bool (*set)(const Device_impl*, Device_state*, int32_t));


/**
 * Register a buffer size set function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param set     The buffer size set function, or \c NULL.
 */
void Device_impl_register_set_buffer_size(
        Device_impl* dimpl,
        bool (*set)(const Device_impl*, Device_state*, int32_t));


/**
 * Register a tempo update function.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param update   The tempo update function, or \c NULL.
 */
void Device_impl_register_update_tempo(
        Device_impl* dimpl,
        void (*update)(const Device_impl*, Device_state*, double));


/**
 * Register a Device state reset function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param reset   The reset function -- must not be \c NULL.
 */
void Device_impl_register_reset_device_state(
        Device_impl* dimpl,
        void (*reset)(const Device_impl*, Device_state*));


/**
 * Register a boolean value set function.
 *
 * The key pattern may contain \c 0 to \c DEVICE_KEY_INDICES_MAX sequences of
 * XX* which are matched against hexadecimal numbers of actual keys. Example:
 *
 *    voice_XX/param_XXX/p_f_volume.json
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
 *    echo_eff/tap_XX/p_f_volume.json <- Invalid: eff is interpreted as hex
 *    echo_XXX/tap_XX/p_f_volume.json <- The above would get confused with this
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_bool(
        Device_impl* dimpl,
        const char* keyp,
        bool default_val,
        bool (*set_func)(
            Device_impl*,
            Device_key_indices,
            bool),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            bool));


/**
 * Register a float value set function.
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
            Device_key_indices,
            double),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            double));


/**
 * Register an integer value set function.
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
            Device_key_indices,
            int64_t),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            int64_t));


/**
 * Register a timestamp value set function.
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
            Device_key_indices,
            const Tstamp*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Tstamp*));


/**
 * Register an envelope value set function.
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
            Device_key_indices,
            const Envelope*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Envelope*));


/**
 * Register a sample value set function.
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
            Device_key_indices,
            const Sample*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Sample*));


/**
 * Register a sample parameters value set function.
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
            Device_key_indices,
            const Sample_params*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Sample_params*));


/**
 * Register a note map value set function.
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
bool Device_impl_register_set_note_map(
        Device_impl* dimpl,
        const char* keyp,
        const Note_map* default_val,
        bool (*set_func)(
            Device_impl*,
            Device_key_indices,
            const Note_map*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Note_map*));


/**
 * Register a hit map value set function.
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
            Device_key_indices,
            const Hit_map*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Hit_map*));


/**
 * Register a number list value set function.
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
            Device_key_indices,
            const Num_list*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Num_list*));


/**
 * Register a boolean value state update function.
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
        void (*update_state)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            bool));


/**
 * Register a float value state update function.
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
        void (*update_state)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            double));


/**
 * Register an integer value state update function.
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
        void (*update_state)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            int64_t));


/**
 * Register a timestamp value state update function.
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
        void (*update_state)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Tstamp*));


/**
 * Reset a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 */
void Device_impl_reset_device_state(
        const Device_impl* dimpl,
        Device_state* dstate);


/**
 * Set the audio rate of a Device state.
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
 * Set the buffer size of a Device state.
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
 * Update the tempo of a Device state.
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
 * Set a key in the Device implementation.
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
 * Notify a Device state of a Device implementation key change.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param key      The key -- must be valid.
 * \param dstate   The Device state -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_set_state_key(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key);


/**
 * Update a boolean value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_update_state_bool(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        bool value);


/**
 * Update a float value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_update_state_float(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        double value);


/**
 * Update a integral value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_update_state_int(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        int64_t value);


/**
 * Update a timestamp value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value -- must not be \c NULL.
 */
void Device_impl_update_state_tstamp(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* value);


/**
 * Deinitialise the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 */
void Device_impl_deinit(Device_impl* dimpl);


/**
 * Destroy the Device implementation.
 *
 * \param dimpl   The Device implementation, or \c NULL.
 */
void del_Device_impl(Device_impl* dimpl);


#endif // K_DEVICE_IMPL_H


