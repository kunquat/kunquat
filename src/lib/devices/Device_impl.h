

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
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


#include <containers/AAtree.h>
#include <Decl.h>
#include <devices/param_types/Envelope.h>
#include <devices/param_types/Hit_map.h>
#include <devices/param_types/Note_map.h>
#include <devices/param_types/Num_list.h>
#include <devices/param_types/Sample.h>
#include <mathnum/Tstamp.h>
#include <player/devices/Device_state.h>
#include <player/Linear_controls.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


// typedefs for value setter callbacks

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

// typedefs for control variable setter callbacks

#define SET_CV_FUNC_TYPE(name, actual_type) \
    typedef void (name)(const Device_impl*, Device_state*, Key_indices, actual_type)
SET_CV_FUNC_TYPE(Set_cv_bool_func,      bool);
SET_CV_FUNC_TYPE(Set_cv_int_func,       int64_t);
SET_CV_FUNC_TYPE(Set_cv_tstamp_func,    const Tstamp*);
#undef SET_CV_FUNC_TYPE

typedef Linear_controls* Get_cv_float_controls_mut_func(
        const Device_impl*, Device_state*, const Key_indices);

// ... and corresponding voice control variable callbacks

#define SET_VOICE_CV_FUNC_TYPE(name, actual_type) \
    typedef void (name)(                          \
            const Device_impl*,                   \
            const Device_state*,                  \
            Voice_state*,                         \
            Key_indices,                          \
            actual_type)
SET_VOICE_CV_FUNC_TYPE(Set_voice_cv_bool_func,      bool);
SET_VOICE_CV_FUNC_TYPE(Set_voice_cv_int_func,       int64_t);
SET_VOICE_CV_FUNC_TYPE(Set_voice_cv_tstamp_func,    const Tstamp*);
#undef SET_VOICE_CV_FUNC_TYPE

typedef Linear_controls* Get_voice_cv_float_controls_mut_func(
        const Device_impl*, const Device_state*, Voice_state*, const Key_indices);


/**
 * The base class of a Processor implementation.
 */
struct Device_impl
{
    Device* device;
    AAtree* set_cbs;
    AAtree* update_cv_cbs;

    bool (*init)(Device_impl*);
    void (*update_tempo)(const Device_impl*, Device_state*, double);
    void (*destroy)(Device_impl*);
};


// Containers of callback functions for control variable types.

typedef struct Device_impl_cv_bool_callbacks
{
    Set_cv_bool_func* set_value;
    Set_voice_cv_bool_func* voice_set_value;
} Device_impl_cv_bool_callbacks;

typedef struct Device_impl_cv_int_callbacks
{
    Set_cv_int_func* set_value;
    Set_voice_cv_int_func* voice_set_value;
} Device_impl_cv_int_callbacks;

typedef struct Device_impl_cv_float_callbacks
{
    Get_cv_float_controls_mut_func* get_controls;
    Get_voice_cv_float_controls_mut_func* get_voice_controls;
} Device_impl_cv_float_callbacks;

typedef struct Device_impl_cv_tstamp_callbacks
{
    Set_cv_tstamp_func* set_value;
    Set_voice_cv_tstamp_func* voice_set_value;
} Device_impl_cv_tstamp_callbacks;


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
 * Register a tempo update function.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param update   The tempo update function, or \c NULL.
 */
void Device_impl_register_update_tempo(
        Device_impl* dimpl,
        void (*update)(const Device_impl*, Device_state*, double));


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
 * Create a boolean control variable.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 *
 * \return   A container for modifier callback functions that modify \a keyp,
 *           or \c NULL if memory allocation failed. All the fields are
 *           initialised to \c NULL. The Device implementation maintains
 *           ownership of the container.
 */
Device_impl_cv_bool_callbacks* Device_impl_create_cv_bool(
        Device_impl* dimpl, const char* keyp);


/**
 * Create an integer control variable.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 *
 * \return   A container for modifier callback functions that modify \a keyp,
 *           or \c NULL if memory allocation failed. All the fields are
 *           initialised to \c NULL. The Device implementation maintains
 *           ownership of the container.
 */
Device_impl_cv_int_callbacks* Device_impl_create_cv_int(
        Device_impl* dimpl, const char* keyp);


/**
 * Create a float control variable.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 *
 * \return   A container for modifier callback functions that modify \a keyp,
 *           or \c NULL if memory allocation failed. All the fields are
 *           initialised to \c NULL. The Device implementation maintains
 *           ownership of the container.
 */
Device_impl_cv_float_callbacks* Device_impl_create_cv_float(
        Device_impl* dimpl, const char* keyp);


/**
 * Create a tstamp control variable.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 *
 * \return   A container for modifier callback functions that modify \a keyp,
 *           or \c NULL if memory allocation failed. All the fields are
 *           initialised to \c NULL. The Device implementation maintains
 *           ownership of the container.
 */
Device_impl_cv_tstamp_callbacks* Device_impl_create_cv_tstamp(
        Device_impl* dimpl, const char* keyp);


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
 * Set a control variable in a Device or Voice state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param vstate   The Voice state, or \c NULL if updating \a dstate.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The Value -- must not be \c NULL and must not have
 *                 \c VALUE_TYPE_FLOAT as type.
 */
void Device_impl_set_cv_generic(
        const Device_impl* dimpl,
        Device_state* dstate,
        Voice_state* vstate,
        const char* key,
        const Value* value);


/**
 * Get modifiable floating-point controls from a Device or Voice state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param vstate   The Voice state, or \c NULL if getting controls from \a dstate.
 * \param key      The key to be updated -- must not be \c NULL.
 *
 * \return   The Linear controls associated with \a key, or \c NULL if \a dimpl does
 *           not support floating-point controls with \a key.
 */
Linear_controls* Device_impl_get_cv_float_controls_mut(
        const Device_impl* dimpl,
        Device_state* dstate,
        Voice_state* vstate,
        const char* key);


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


