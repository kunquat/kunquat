

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
#include <string/key_pattern.h>
#include <Tstamp.h>


// typedefs for value setter/updater callbacks

#define SET_FUNC_TYPE(name, actual_type)                                          \
    typedef bool (Set_ ## name ## _func)(Device_impl*, Key_indices, actual_type); \
    typedef bool (Set_state_ ## name ## _func)(                                   \
            const Device_impl*, Device_state*, Key_indices, actual_type)
SET_FUNC_TYPE(bool,             bool);
SET_FUNC_TYPE(int,              int64_t);
SET_FUNC_TYPE(float,            double);
SET_FUNC_TYPE(tstamp,           const Tstamp*);
SET_FUNC_TYPE(envelope,         const Envelope*);
SET_FUNC_TYPE(sample,           const Sample*);
SET_FUNC_TYPE(sample_params,    const Sample_params*);
SET_FUNC_TYPE(note_map,         const Note_map*);
SET_FUNC_TYPE(hit_map,          const Hit_map*);
SET_FUNC_TYPE(num_list,         const Num_list*);
#undef SET_FUNC_TYPE

#define UPDATE_FUNC_TYPE(name, actual_type)                              \
    typedef void (Update_ ## name ## _func)(                             \
            const Device_impl*, Device_state*, Key_indices, actual_type)
UPDATE_FUNC_TYPE(bool,      bool);
UPDATE_FUNC_TYPE(int,       int64_t);
UPDATE_FUNC_TYPE(float,     double);
UPDATE_FUNC_TYPE(tstamp,    const Tstamp*);
#undef UPDATE_FUNC_TYPE


/**
 * The base class of Generator and DSP implementations.
 */
struct Device_impl
{
    Device* device;
    AAtree* set_cbs;
    AAtree* update_state_cbs;

    bool (*init)(Device_impl*);
    bool (*set_audio_rate)(const Device_impl*, Device_state*, int32_t);
    bool (*set_buffer_size)(const Device_impl*, Device_state*, int32_t);
    void (*update_tempo)(const Device_impl*, Device_state*, double);
    void (*reset)(const Device_impl*, Device_state*);
    void (*destroy)(Device_impl*);
};


/**
 * Register an initialisation function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param init    The initialisation function -- must not be \c NULL.
 */
void Device_impl_register_init(Device_impl* dimpl, bool (*init)(Device_impl*));


/**
 * Register a destructor.
 *
 * \param dimpl     The Device implementation -- must not be \c NULL.
 * \param destroy   The destructor -- must not be \c NULL.
 */
void Device_impl_register_destroy(Device_impl* dimpl, void (*destroy)(Device_impl*));


/**
 * Initialise the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_init(Device_impl* dimpl);


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
        Set_bool_func set_func,
        Set_state_bool_func set_state_func);


/**
 * Register a float value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_float(
        Device_impl* dimpl,
        const char* keyp,
        double default_val,
        Set_float_func set_func,
        Set_state_float_func set_state_func);


/**
 * Register an integer value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_int(
        Device_impl* dimpl,
        const char* keyp,
        int64_t default_val,
        Set_int_func set_func,
        Set_state_int_func set_state_func);


/**
 * Register a timestamp value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        const Tstamp* default_val,
        Set_tstamp_func set_func,
        Set_state_tstamp_func set_state_func);


/**
 * Register an envelope value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_envelope(
        Device_impl* dimpl,
        const char* keyp,
        const Envelope* default_val,
        Set_envelope_func set_func,
        Set_state_envelope_func set_state_func);


/**
 * Register a sample value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_sample(
        Device_impl* dimpl,
        const char* keyp,
        const Sample* default_val,
        Set_sample_func set_func,
        Set_state_sample_func set_state_func);


/**
 * Register a sample parameters value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_sample_params(
        Device_impl* dimpl,
        const char* keyp,
        const Sample_params* default_val,
        Set_sample_params_func set_func,
        Set_state_sample_params_func set_state_func);


/**
 * Register a note map value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_note_map(
        Device_impl* dimpl,
        const char* keyp,
        const Note_map* default_val,
        Set_note_map_func set_func,
        Set_state_note_map_func set_state_func);


/**
 * Register a hit map value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_hit_map(
        Device_impl* dimpl,
        const char* keyp,
        const Hit_map* default_val,
        Set_hit_map_func set_func,
        Set_state_hit_map_func set_state_func);


/**
 * Register a number list value set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl            The Device implementation -- must not be \c NULL.
 * \param keyp             The key pattern -- must not be \c NULL.
 * \param default_val      The default value passed to callbacks
 *                         -- must not be \c NULL.
 * \param set_func         The set function -- must not be \c NULL.
 * \param set_state_func   The set state function, or \c NULL if not used.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_num_list(
        Device_impl* dimpl,
        const char* keyp,
        const Num_list* default_val,
        Set_num_list_func set_func,
        Set_state_num_list_func set_state_func);


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
        Device_impl* dimpl, const char* keyp, Update_bool_func update_state);


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
        Device_impl* dimpl, const char* keyp, Update_float_func update_state);


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
        Device_impl* dimpl, const char* keyp, Update_int_func update_state);


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
        Device_impl* dimpl, const char* keyp, Update_tstamp_func update_state);


/**
 * Reset a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 */
void Device_impl_reset_device_state(
        const Device_impl* dimpl, Device_state* dstate);


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
        const Device_impl* dimpl, Device_state* dstate, int32_t audio_rate);


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
        const Device_impl* dimpl, Device_state* dstate, int32_t buffer_size);


/**
 * Update the tempo of a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param tempo    The new tempo -- must be finite and > \c 0.
 */
void Device_impl_update_tempo(
        const Device_impl* dimpl, Device_state* dstate, double tempo);


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
        const Device_impl* dimpl, Device_state* dstate, const char* key);


/**
 * Update a boolean value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_update_state_bool(
        const Device_impl* dimpl, Device_state* dstate, const char* key, bool value);


/**
 * Update a float value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_update_state_float(
        const Device_impl* dimpl, Device_state* dstate, const char* key, double value);


/**
 * Update a integral value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_update_state_int(
        const Device_impl* dimpl, Device_state* dstate, const char* key, int64_t value);


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


