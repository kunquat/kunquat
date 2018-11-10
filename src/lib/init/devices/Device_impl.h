

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
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
#include <init/devices/Device_port_groups.h>
#include <init/devices/param_types/Envelope.h>
#include <init/devices/param_types/Hit_map.h>
#include <init/devices/param_types/Note_map.h>
#include <init/devices/param_types/Num_list.h>
#include <init/devices/param_types/Padsynth_params.h>
#include <init/devices/param_types/Sample.h>
#include <init/devices/port_type.h>
#include <init/devices/Proc_type.h>
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
//SET_FUNC_TYPE(padsynth_params,  const Padsynth_params*);
#undef SET_FUNC_TYPE

typedef bool (Set_padsynth_params_func)(
        Device_impl*, const Key_indices, const Padsynth_params*, Background_loader*);
typedef bool (Set_state_padsynth_params_func)(
        Device_state*, const Key_indices, const Padsynth_params*);


typedef void Device_impl_get_port_groups_func(
        const Device_impl*, Device_port_type, Device_port_groups);
typedef int32_t Device_impl_get_voice_wb_size_func(
        const Device_impl*, int32_t audio_rate);
typedef void Device_impl_destroy_func(Device_impl*);


/**
 * The base class of a Processor implementation.
 */
struct Device_impl
{
    const Device* device;
    AAtree* set_cbs;

    Proc_type proc_type;

    Device_impl_get_port_groups_func* get_port_groups;
    Device_state_create_func* create_pstate;
    Voice_state_get_size_func* get_vstate_size;
    Device_impl_get_voice_wb_size_func* get_voice_wb_size;
    Voice_state_init_func* init_vstate;
    Voice_state_render_voice_func* render_voice;
    Voice_state_fire_event_func* fire_voice_dev_event;
    Device_impl_destroy_func* destroy;
};



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
 * Set the Processor type of the Device implementation.
 *
 * \param dimpl       The Device implementation -- must not be \c NULL.
 * \param proc_type   The Processor type -- must be valid.
 */
void Device_impl_set_proc_type(Device_impl* dimpl, Proc_type proc_type);


/**
 * Get the Processor type of the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 *
 * \return   The Processor type.
 */
Proc_type Device_impl_get_proc_type(const Device_impl* dimpl);


/**
 * Set the Device of the Device implementation.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param device   The Device -- must not be \c NULL.
 */
void Device_impl_set_device(Device_impl* dimpl, const Device* device);


/**
 * Get Device implementation port group information.
 *
 * \param dimpl    The Device implementation -- must not be \c NULL.
 * \param type     The port type -- must be a valid type.
 * \param groups   Destination where the port groups will be stored -- must not
 *                 be \c NULL.
 */
void Device_impl_get_port_groups(
        const Device_impl* dimpl, Device_port_type type, Device_port_groups groups);


/**
 * Get Voice state size required by the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 *
 * \return   The size required for Voice states of this Device implementation.
 */
int32_t Device_impl_get_vstate_size(const Device_impl* dimpl);


/**
 * Get Voice work buffer size required by the Device implementation.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be > \c 0.
 *
 * \return   The buffer size required.
 */
int32_t Device_impl_get_voice_wb_size(const Device_impl* dimpl, int32_t audio_rate);


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
 * Set a key in the Device implementation.
 *
 * The actual data is retrieved from device parameters.
 *
 * \param dimpl        The Device implementation -- must not be \c NULL.
 * \param key          The key -- must be valid.
 * \param bkg_loader   The Background loader -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_set_key(
        Device_impl* dimpl, const char* key, Background_loader* bkg_loader);


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


