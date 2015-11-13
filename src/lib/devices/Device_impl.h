

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

#define SET_CV_FUNC_TYPE(name, actual_type)                              \
    typedef void (Set_cv_ ## name ## _func)(                             \
            const Device_impl*, Device_state*, Key_indices, actual_type)
SET_CV_FUNC_TYPE(bool,      bool);
SET_CV_FUNC_TYPE(int,       int64_t);
SET_CV_FUNC_TYPE(float,     double);
SET_CV_FUNC_TYPE(tstamp,    const Tstamp*);
#undef SET_CV_FUNC_TYPE

// typedefs for floating-point control variable slider callbacks

typedef void (Slide_target_cv_float_func)(
        const Device_impl*, Device_state*, Key_indices, double);
typedef void (Slide_length_cv_float_func)(
        const Device_impl*, Device_state*, Key_indices, const Tstamp*);

// typedefs for floating-point control variable oscillation callbacks

typedef void (Osc_speed_cv_float_func)(
        const Device_impl*, Device_state*, Key_indices, double);
typedef void (Osc_depth_cv_float_func)(
        const Device_impl*, Device_state*, Key_indices, double);
typedef void (Osc_speed_slide_cv_float_func)(
        const Device_impl*, Device_state*, Key_indices, const Tstamp*);
typedef void (Osc_depth_slide_cv_float_func)(
        const Device_impl*, Device_state*, Key_indices, const Tstamp*);


/**
 * The base class of a Processor implementation.
 */
struct Device_impl
{
    Device* device;
    AAtree* set_cbs;
    AAtree* update_cv_cbs;

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
 * Register a boolean control variable set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 * \param set_cv   The control variable set callback function
 *                 -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_cv_bool(
        Device_impl* dimpl, const char* keyp, Set_cv_bool_func set_cv);


/**
 * Register floating-point control variable update functions.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param set_cv         The control variable set callback function, or \c NULL.
 * \param slide_target   The slide target function, or \c NULL.
 * \param slide_length   The slide length function, or \c NULL.
 * \param osc_speed      The oscillation speed function, or \c NULL.
 * \param osc_depth      The oscillation depth function, or \c NULL.
 * \param osc_speed_sl   The oscillation speed slide length function, or \c NULL.
 * \param osc_depth_sl   The oscillation depth slide length function, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_updaters_cv_float(
        Device_impl* dimpl,
        const char* keyp,
        Set_cv_float_func set_cv,
        Slide_target_cv_float_func slide_target,
        Slide_length_cv_float_func slide_length,
        Osc_speed_cv_float_func osc_speed,
        Osc_depth_cv_float_func osc_depth,
        Osc_speed_slide_cv_float_func osc_speed_sl,
        Osc_depth_slide_cv_float_func osc_depth_sl);


/**
 * Register an integer control variable set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 * \param set_cv   The control variable set callback function
 *                 -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_cv_int(
        Device_impl* dimpl, const char* keyp, Set_cv_int_func set_cv);


/**
 * Register a timestamp control variable set function.
 *
 * See \a Device_impl_register_set_bool for a detailed description of the
 * \a keyp argument.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param keyp     The key pattern -- must not be \c NULL.
 * \param set_cv   The control variable set callback function
 *                 -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_set_cv_tstamp(
        Device_impl* dimpl, const char* keyp, Set_cv_tstamp_func set_cv);


/**
 * Reset a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 */
void Device_impl_reset_device_state(const Device_impl* dimpl, Device_state* dstate);


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
 * Set a boolean control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_set_cv_bool(
        const Device_impl* dimpl, Device_state* dstate, const char* key, bool value);


/**
 * Set a float control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_set_cv_float(
        const Device_impl* dimpl, Device_state* dstate, const char* key, double value);


/**
 * Slide float control variable to target value in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The target value.
 */
void Device_impl_slide_cv_float_target(
        const Device_impl* dimpl, Device_state* dstate, const char* key, double value);


/**
 * Set slide length of float control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param length   The slide length -- must not be \c NULL.
 */
void Device_impl_slide_cv_float_length(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* length);


/**
 * Set oscillation speed of float control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param speed    The oscillation speed -- must be >= \c 0.
 */
void Device_impl_osc_speed_cv_float(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        double speed);


/**
 * Set oscillation depth of float control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param depth    The oscillation depth -- must be >= \c 0.
 */
void Device_impl_osc_depth_cv_float(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        double depth);


/**
 * Set oscillation speed slide of float control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param length   The slide length -- must be >= \c 0.
 */
void Device_impl_osc_speed_slide_cv_float(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* length);


/**
 * Set oscillation depth slide of float control variable in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param length   The slide length -- must be >= \c 0.
 */
void Device_impl_osc_depth_slide_cv_float(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* length);


/**
 * Set an integral control in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value.
 */
void Device_impl_set_cv_int(
        const Device_impl* dimpl, Device_state* dstate, const char* key, int64_t value);


/**
 * Set a timestamp control in a Device state.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key to be updated -- must not be \c NULL.
 * \param value    The value -- must not be \c NULL.
 */
void Device_impl_set_cv_tstamp(
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


