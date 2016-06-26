

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_IMPL_H
#define KQT_DEVICE_IMPL_H


#include <containers/AAtree.h>
#include <decl.h>
#include <init/devices/param_types/Envelope.h>
#include <init/devices/param_types/Hit_map.h>
#include <init/devices/param_types/Note_map.h>
#include <init/devices/param_types/Num_list.h>
#include <init/devices/param_types/Padsynth_params.h>
#include <init/devices/param_types/Sample.h>
#include <mathnum/Tstamp.h>
#include <player/devices/Device_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>
#include <string/key_pattern.h>
#include <Value.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


// typedefs for value setter callbacks

#define SET_FUNC_TYPE(name, param_type)                                                \
    typedef bool (Set_ ## name ## _func)(Device_impl*, const Key_indices, param_type); \
    typedef bool (Set_state_ ## name ## _func)(                                        \
            Device_state*, const Key_indices, param_type)
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
SET_FUNC_TYPE(padsynth_params,  const Padsynth_params*);
#undef SET_FUNC_TYPE


typedef void Device_impl_destroy_func(Device_impl*);


/**
 * The base class of a Processor implementation.
 */
struct Device_impl
{
    const Device* device;
    AAtree* set_cbs;
    AAtree* update_cv_cbs;

    Device_state_create_func* create_pstate;
    Voice_state_get_size_func* get_vstate_size;
    Voice_state_init_func* init_vstate;
    Device_impl_destroy_func* destroy;
};


// Containers of callback functions for control variable types.

typedef struct Device_impl_proc_cv_callback
{
    Value_type type;
    Key_indices indices;
    union
    {
        Proc_state_set_cv_bool_func* set_bool;
        Proc_state_set_cv_int_func* set_int;
        Proc_state_set_cv_float_func* set_float;
        Proc_state_set_cv_tstamp_func* set_tstamp;
    } cb;
} Device_impl_proc_cv_callback;

#define DEVICE_IMPL_PROC_CV_CALLBACK_AUTO \
    (&(Device_impl_proc_cv_callback){ .type = VALUE_TYPE_NONE })

typedef struct Device_impl_voice_cv_callback
{
    Value_type type;
    Key_indices indices;
    union
    {
        Voice_state_set_cv_bool_func* set_bool;
        Voice_state_set_cv_int_func* set_int;
        Voice_state_set_cv_float_func* set_float;
        Voice_state_set_cv_tstamp_func* set_tstamp;
    } cb;
} Device_impl_voice_cv_callback;

#define DEVICE_IMPL_VOICE_CV_CALLBACK_AUTO \
    (&(Device_impl_voice_cv_callback){ .type = VALUE_TYPE_NONE })


/**
 * Initialise the Device implementation.
 *
 * \param dimpl     The Device implementation -- must not be \c NULL.
 * \param destroy   The destructor function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 *           The Device implementation class allows calling \a del_Device_impl
 *           even if the initialisation fails, as long as the derived class
 *           handles its own partially initialised state correctly.
 */
bool Device_impl_init(Device_impl* dimpl, Device_impl_destroy_func* destroy);


/**
 * Set the Device of the Device implementation.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param device   The Device -- must not be \c NULL.
 */
void Device_impl_set_device(Device_impl* dimpl, const Device* device);


/**
 * Get Voice state size required by the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 *
 * \return   The size required for Voice states of this Device implementation.
 */
size_t Device_impl_get_vstate_size(const Device_impl* dimpl);


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
 * Register a PADsynth parameters value set function.
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
bool Device_impl_register_set_padsynth_params(
        Device_impl* dimpl,
        const char* keyp,
        const Padsynth_params* default_val,
        Set_padsynth_params_func set_func,
        Set_state_padsynth_params_func set_state_func);


/**
 * Create a boolean control variable.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param keyp         The key pattern -- must not be \c NULL.
 * \param pstate_set   The Processor state set function, or \c NULL.
 * \param vstate_set   The Voice state set function, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_create_cv_bool(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_bool_func* pstate_set,
        Voice_state_set_cv_bool_func* vstate_set);


/**
 * Create an integer control variable.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param keyp         The key pattern -- must not be \c NULL.
 * \param pstate_set   The Processor state set function, or \c NULL.
 * \param vstate_set   The Voice state set function, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_create_cv_int(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_int_func* pstate_set,
        Voice_state_set_cv_int_func* vstate_set);


/**
 * Create a float control variable.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param keyp         The key pattern -- must not be \c NULL.
 * \param pstate_get   The Processor state Linear controls get function, or \c NULL.
 * \param vstate_get   The Voice state Linear controls get function, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_create_cv_float(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_float_func* pstate_set,
        Voice_state_set_cv_float_func* vstate_set);


/**
 * Create a tstamp control variable.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param keyp         The key pattern -- must not be \c NULL.
 * \param pstate_set   The Processor state set function, or \c NULL.
 * \param vstate_set   The Voice state set function, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_create_cv_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_tstamp_func* pstate_set,
        Voice_state_set_cv_tstamp_func* vstate_set);


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
 * Get a Processor control variable callback function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param key     The key of the control variable -- must not be \c NULL.
 * \param type    The control variable type.
 * \param cb      The destination address of the callback structure -- must
 *                not be \c NULL.
 */
void Device_impl_get_proc_cv_callback(
        const Device_impl* dimpl,
        const char* key,
        Value_type type,
        Device_impl_proc_cv_callback* cb);


/**
 * Get a Voice control variable callback function.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 * \param key     The key of the control variable -- must not be \c NULL.
 * \param type    The control variable type.
 * \param cb      The destination address of the callback structure -- must
 *                not be \c NULL.
 */
void Device_impl_get_voice_cv_callback(
        const Device_impl* dimpl,
        const char* key,
        Value_type type,
        Device_impl_voice_cv_callback* cb);


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


#endif // KQT_DEVICE_IMPL_H


